// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shooter_Online/Weapon//Weapon.h"
#include "Shooter_Online/Shooter_Components/Combat_Component.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AShooter_Character::AShooter_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Camera_Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera_Boom"));
	Camera_Boom->SetupAttachment(GetMesh());
	Camera_Boom->TargetArmLength = 555.f;
	Camera_Boom->bUsePawnControlRotation = true;


	Follow_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow_Camera"));
	Follow_Camera->SetupAttachment(Camera_Boom, USpringArmComponent::SocketName);
	Follow_Camera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	Overhead_Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead_Widget"));
	Overhead_Widget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombat_Component>(TEXT("Combat_Component"));
	Combat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

}

void AShooter_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooter_Character, Overlapping_Weapon, COND_OwnerOnly);
}

// Called when the game starts or when spawned
void AShooter_Character::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AShooter_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Aim_Offset(DeltaTime);

}

// Called to bind functionality to input
void AShooter_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAxis("Move_Forward", this, &ThisClass::Move_Forward);
	PlayerInputComponent->BindAxis("Move_Right", this, &ThisClass::Move_Right);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("Look_Up", this, &ThisClass::Look_Up);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShooter_Character::Equip_Button_Pressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooter_Character::Crouch_Button_Pressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AShooter_Character::Aim_Button_Pressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AShooter_Character::Aim_Button_Released);
}

void AShooter_Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character = this;
	}
}

#pragma region Functions

void AShooter_Character::Move_Forward(float Value)
{
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator Yaw_Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(Yaw_Rotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AShooter_Character::Move_Right(float Value)
{
	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator Yaw_Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(Yaw_Rotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AShooter_Character::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AShooter_Character::Look_Up(float Value)
{
	AddControllerPitchInput(Value);
}

void AShooter_Character::Equip_Button_Pressed()
{
	if(Combat)
	{
		if(HasAuthority())
		{
			Combat->Equip_Weapon(Overlapping_Weapon);
		}
		else
		{
			Server_Equip_Button_Pressed();
		}
	}
}

void AShooter_Character::Server_Equip_Button_Pressed_Implementation()
{
	if(Combat)
	{
		Combat->Equip_Weapon(Overlapping_Weapon);
	}
}

void AShooter_Character::Crouch_Button_Pressed()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AShooter_Character::Aim_Button_Pressed()
{
	if(Combat)
	{
		Combat->Set_Aiming(true);
	}
}

void AShooter_Character::Aim_Button_Released()
{
	if(Combat)
	{
		Combat->Set_Aiming(false);
	}
}

void AShooter_Character::Aim_Offset(float DeltaTime)
{
	if(Combat && Combat->Equipped_Weapon == nullptr) return;
	FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    float Speed = Velocity.Size();
	bool bIs_In_Air = GetCharacterMovement()->IsFalling();

	if(Speed == 0.f && !bIs_In_Air) //Standing still and not jumping.
	{
		FRotator Current_Aim_Rotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator Delta_Aim_Rotation = UKismetMathLibrary::NormalizedDeltaRotator(Current_Aim_Rotation, Starting_Aim_Rotation);
		AO_Yaw = Delta_Aim_Rotation.Yaw;
		bUseControllerRotationYaw = false;
	}
	if(Speed > 0.f || bIs_In_Air) // Running or jumping.
	{
		Starting_Aim_Rotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		//map pitch from [270, 360) to [-90, 0)
		FVector2D In_Range(270.f, 360.f);
		FVector2D Out_Range(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(In_Range, Out_Range, AO_Pitch);
	}
	
	//Used to fix pitch on server character
	/*if(!HasAuthority() && !IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AO Pitch %f"), AO_Pitch);
	}*/
}

void AShooter_Character::Set_Overlapping_Weapon(AWeapon* Weapon)
{
	if(Overlapping_Weapon != nullptr)
		{
			Overlapping_Weapon->Show_Pickup_Widget(false);
		}
	Overlapping_Weapon = Weapon;
	if(IsLocallyControlled())
	{
		if(Overlapping_Weapon != nullptr)
		{
			Overlapping_Weapon->Show_Pickup_Widget(true);
		}

	}
}

void AShooter_Character::OnRep_Overlapping_Weapon(AWeapon* Last_Weapon)
{
	if(Overlapping_Weapon != nullptr)
	{
		Overlapping_Weapon->Show_Pickup_Widget(true);
	}
	if(Last_Weapon != nullptr)
	{
		Last_Weapon->Show_Pickup_Widget(false);
	}
}

bool AShooter_Character::Is_Weapon_Equipped()
{
    return (Combat && Combat->Equipped_Weapon);
}

bool AShooter_Character::Is_Aiming()
{
    return (Combat && Combat->bAiming);
}

AWeapon *AShooter_Character::Get_Equipped_Weapon()
{
   if(Combat == nullptr) return nullptr;
	return Combat->Equipped_Weapon;
}

#pragma endregion