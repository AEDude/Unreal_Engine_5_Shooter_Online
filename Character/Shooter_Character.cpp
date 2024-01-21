// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shooter_Online/Weapon//Weapon.h"
#include "Shooter_Online/Shooter_Components/Combat_Component.h"
#include "Shooter_Online/Shooter_Components/Buff_Component.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shooter_Anim_Instance.h"
#include "Shooter_Online/Shooter_Online.h"
#include "Shooter_Online/Player_Controller/Shooter_Player_Controller.h"
#include "Shooter_Online/Game_Mode/Shooter_Online_Game_Mode.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Shooter_Online/Player_State/Shooter_Player_State.h"
#include "Shooter_Online/Weapon/Weapon_Types.h"
#include "Kismet/GameplayStatics.h"


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

	Buff = CreateDefaultSubobject<UBuff_Component>(TEXT("Buff_Component"));
	Buff->SetIsReplicated(true);

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	Attached_Grenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached_Grenade"));
	Attached_Grenade->SetupAttachment(GetMesh(), FName("Grenade_Socket"));
	Attached_Grenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_Skeletal_Mesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
	NetUpdateFrequency = 70.f;
	MinNetUpdateFrequency = 33.f;

}


// Called when the game starts or when spawned
void AShooter_Character::BeginPlay()
{
	Super::BeginPlay();

	Update_HUD_Health();
    Update_HUD_Armor();
	Update_HUD_Ammo();

	if(Combat)
	{
		Combat->Spawn_Default_Weapon();
	}

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AShooter_Character::Recieve_Damage);
	}
	
	if(Attached_Grenade)
	{
		Attached_Grenade->SetVisibility(false);
	}
	
}

// Called every frame
void AShooter_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Rotate_In_Place(DeltaTime);

	Hide_Camera_When_Character_Is_Close();
	Poll_Initialize();
}

// Called to bind functionality to input
void AShooter_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooter_Character::Jump);

	PlayerInputComponent->BindAxis("Move_Forward", this, &ThisClass::Move_Forward);
	PlayerInputComponent->BindAxis("Move_Right", this, &ThisClass::Move_Right);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("Look_Up", this, &ThisClass::Look_Up);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShooter_Character::Equip_Button_Pressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooter_Character::Crouch_Button_Pressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AShooter_Character::Aim_Button_Pressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AShooter_Character::Aim_Button_Released);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooter_Character::Fire_Button_Pressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooter_Character::Fire_Button_Released);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooter_Character::Reload_Button_Pressed);
	PlayerInputComponent->BindAction("Throw_Grenade", IE_Released, this, &AShooter_Character::Grenade_Button_Pressed);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AShooter_Character::Sprint_Button_Pressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AShooter_Character::Sprint_Button_Released);
}

void AShooter_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooter_Character, Overlapping_Weapon, COND_OwnerOnly);
	DOREPLIFETIME(AShooter_Character, Health);
	DOREPLIFETIME(AShooter_Character, Armor);
	DOREPLIFETIME(AShooter_Character, bDisable_Gameplay);
	
}

void AShooter_Character::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	Simulated_Proxies_Turn();
	Time_Since_Last_Movement_Replication = 0.f;
}

void AShooter_Character::Eliminated()
{
	Drop_Or_Destroy_Weapons();
	Multicast_Eliminated();
	GetWorldTimerManager().SetTimer(
		Eliminated_Timer,
		this,
		&AShooter_Character::Eliminated_Timer_Finished,
		Eliminated_Delay
	);
}

void AShooter_Character::Multicast_Eliminated_Implementation()
{	
	if(Shooter_Player_Controller)
	{
		Shooter_Player_Controller->Set_HUD_Weapon_Ammo(0);
	}
	bEliminated = true;
	Ragdoll_On_Death();
	//Play_Eliminated_Montage();

	//Disable All Controller Input
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if(Shooter_Player_Controller && Combat)
	{
		DisableInput(Shooter_Player_Controller);
		bDisable_Gameplay = true;
		Combat->Fire_Button_Pressed(false);
	}
	bool bHide_Sniper_Scope = IsLocallyControlled() && 
	Combat && 
	Combat->bAiming && 
	Combat->Equipped_Weapon && 
	Combat->Equipped_Weapon->Get_Weapon_Type() == EWeapon_Type::EWT_Sniper_Rifle;
	if(bHide_Sniper_Scope)
	{
		Show_Sniper_Scope_Widget(false);
	}
	
	/*Dsiable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);*/
}

void AShooter_Character::Eliminated_Timer_Finished()
{
	AShooter_Online_Game_Mode* Shooter_Online_Game_Mode = GetWorld()->GetAuthGameMode<AShooter_Online_Game_Mode>();
	if(Shooter_Online_Game_Mode)
	{
		Shooter_Online_Game_Mode->Request_Respawn(this, Controller);
	}
}

void AShooter_Character::Drop_Or_Destroy_Weapons()
{
	if(Combat)
	{
		if(Combat->Equipped_Weapon)
		{
			Drop_Or_Destroy_Weapon(Combat->Equipped_Weapon);
		}
		if(Combat->Secondary_Weapon)
		{
			Drop_Or_Destroy_Weapon(Combat->Secondary_Weapon);
		}
	}
}

void AShooter_Character::Drop_Or_Destroy_Weapon(AWeapon* Weapon)
{
	if(Weapon == nullptr) return;
	if(Weapon->bDestroy_Weapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Drop_Weapons();
	}
}

void AShooter_Character::Destroyed()
{
	Super::Destroyed();

	AShooter_Online_Game_Mode* Shooter_Game_Mode = Cast<AShooter_Online_Game_Mode>(UGameplayStatics::GetGameMode(this));
	bool bMatch_Is_not_In_Progress = Shooter_Game_Mode && Shooter_Game_Mode->GetMatchState() != MatchState::InProgress;
	if(Combat && Combat->Equipped_Weapon && bMatch_Is_not_In_Progress)
	{
		Combat->Equipped_Weapon->Destroy();
	}
}

void AShooter_Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if(Combat)
	{
		Combat->Character = this;
	}
	if(Buff)
	{
		Buff->Character = this;
		Buff->Set_Initial_Speeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
	}
}

void AShooter_Character::Play_Fire_Montage(bool bAiming)
{
	if(Combat == nullptr || Combat->Equipped_Weapon == nullptr) return;

	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Fire_Weapon_Montage)
	{
		Anim_Instance->Montage_Play(Fire_Weapon_Montage);
		FName Section_Name;
		Section_Name = bAiming ? FName("Rifle_Aim") : FName("Rifle_Hip");
		Anim_Instance->Montage_JumpToSection(Section_Name);
	}
}

void AShooter_Character::Play_Reload_Montage()
{
	if(Combat == nullptr || Combat->Equipped_Weapon == nullptr) return;

	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Reload_Montage)
	{
		Anim_Instance->Montage_Play(Reload_Montage);
		FName Section_Name;
		
		switch(Combat->Equipped_Weapon->Get_Weapon_Type())
		{
		case EWeapon_Type::EWT_Pistol:
			Section_Name = FName("Pistol");
			break;
		case EWeapon_Type::EWT_Submachine_Gun:
			Section_Name = FName("Submachine_Gun");
			break;
		case EWeapon_Type::EWT_Assult_Rifle:
			Section_Name = FName("Assult_Rifle");
			break;
		case EWeapon_Type::EWT_Shotgun:
			Section_Name = FName("Shotgun");
			break;
		case EWeapon_Type::EWT_Sniper_Rifle:
			Section_Name = FName("Rifle");
			break;
		case EWeapon_Type::EWT_Grenade_Launcher:
			Section_Name = FName("Assult_Rifle");
			break;	
		case EWeapon_Type::EWT_Rocket_Launcher:
			Section_Name = FName("Assult_Rifle");
			break;
		}

		Anim_Instance->Montage_JumpToSection(Section_Name);
	}
}

void AShooter_Character::Play_Hit_React_Montage()
{
	if(Combat == nullptr || Combat->Equipped_Weapon == nullptr) return;

	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Hit_React_Montage)
	{
		Anim_Instance->Montage_Play(Hit_React_Montage);
		FName Section_Name("From_Front");
		Anim_Instance->Montage_JumpToSection(Section_Name);
	}
}

void AShooter_Character:: Grenade_Button_Pressed()
{
	if(Combat)
	{
		Combat->Throw_Grenade();
	}
}

void AShooter_Character::Play_Eliminated_Montage()
{
	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Eliminated_Montage)
	{
		Anim_Instance->Montage_Play(Eliminated_Montage);
		/*FName Section_Name;
		Section_Name = bAiming ? FName("Rifle_Aim") : FName("Rifle_Hip");
		Anim_Instance->Montage_JumpToSection(Section_Name);*/
	}
}

void AShooter_Character::Play_Throw_Grenade_Montage()
{
	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Throw_Grenade_Montage)
	{
		Anim_Instance->Montage_Play(Throw_Grenade_Montage);
	}
}

void AShooter_Character::Ragdoll_On_Death()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetSimulatePhysics(true);
}

void AShooter_Character::Recieve_Damage(AActor *Damaged_Actor, float Damage, const UDamageType *Damage_Type, AController *Instigator_Controller, AActor *Damage_Causer)
{
	if(bEliminated) return;

	float Damage_To_Health = Damage;
	if(Armor > 0.f)
	{
		if(Armor >= Damage)
		{
			Armor = FMath::Clamp(Armor - Damage, 0.f, Max_Armor);
			Damage_To_Health = 0.f;
		}
		else
		{
			Damage_To_Health = FMath::Clamp(Damage_To_Health - Armor, 0.f, Damage);
			Armor = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - Damage_To_Health, 0.f, Max_Health);
	
	Update_HUD_Health();
	Update_HUD_Armor();
	Play_Hit_React_Montage();
	if(Health == 0.f)
	{
		AShooter_Online_Game_Mode* Shooter_Online_Game_Mode = GetWorld()->GetAuthGameMode<AShooter_Online_Game_Mode>();
		if(Shooter_Online_Game_Mode)
		{	
			Shooter_Player_Controller = Shooter_Player_Controller == nullptr ? Cast<AShooter_Player_Controller>(Controller) : Shooter_Player_Controller;
			AShooter_Player_Controller* Attacker_Controller = Cast<AShooter_Player_Controller>(Instigator_Controller);
			Shooter_Online_Game_Mode->Player_Eliminated(this, Shooter_Player_Controller, Attacker_Controller);
		}
	}
	
}

void AShooter_Character::Move_Forward(float Value)
{
	if(bDisable_Gameplay) return;

	if(Controller != nullptr && Value != 0.f)
	{
		const FRotator Yaw_Rotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(Yaw_Rotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AShooter_Character::Move_Right(float Value)
{
	if(bDisable_Gameplay) return;

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
	if(bDisable_Gameplay) return;
	if(Combat)
	{
		Server_Equip_Button_Pressed();
	}
}

void AShooter_Character::Server_Equip_Button_Pressed_Implementation()
{
	if(Combat)
	{
		if(Overlapping_Weapon)
		{
			Combat->Equip_Weapon(Overlapping_Weapon);
		}
		else if (Combat->Should_Swap_Weapons())
		{
			Combat->Swap_Weapons();
		}
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

void AShooter_Character::Reload_Button_Pressed()
{	
	if(bDisable_Gameplay) return;

	if(Combat && !Combat->bSprinting)
	{
		Combat->Reload();
	}
}

void AShooter_Character::Aim_Button_Pressed()
{
	if(Combat && !Combat->bSprinting && Get_Combat_State() != ECombat_State::ECS_Reloading)
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

float AShooter_Character::Calculate_Speed()
{
	FVector Velocity = GetVelocity();
    Velocity.Z = 0.f;
    return Velocity.Size();
}

void AShooter_Character::Aim_Offset(float DeltaTime)
{
	if(Combat && Combat->Equipped_Weapon == nullptr || Combat->bSprinting == true) return;
    float Speed = Calculate_Speed();
	bool bIs_In_Air = GetCharacterMovement()->IsFalling();

	if(Speed == 0.f && !bIs_In_Air) //Standing still and not jumping.
	{
		bRotate_Root_Bone = true;
		FRotator Current_Aim_Rotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator Delta_Aim_Rotation = UKismetMathLibrary::NormalizedDeltaRotator(Current_Aim_Rotation, Starting_Aim_Rotation);
		AO_Yaw = Delta_Aim_Rotation.Yaw;
		if(Turning_In_Place == ETurning_In_Place::ETIP_Not_Turning)
		{
			Interp_AO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;

		//Turning in place.
		Turn_In_Place(DeltaTime);
	}
	if(Speed > 0.f || bIs_In_Air) // Running or jumping.
	{
		bRotate_Root_Bone = false;
		Starting_Aim_Rotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
	}

	Calculate_AO_Pitch();
}

void AShooter_Character::Rotate_In_Place(float DeltaTime)
{
	if(GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		Aim_Offset(DeltaTime);
	}
	else
	{
		Time_Since_Last_Movement_Replication += DeltaTime;
		if(Time_Since_Last_Movement_Replication > 0.3f)
		{
			OnRep_ReplicatedMovement();
		}
		Calculate_AO_Pitch();
	}
}

void AShooter_Character::Calculate_AO_Pitch()
{
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

void AShooter_Character::Simulated_Proxies_Turn()
{
	if(Combat == nullptr || Combat->Equipped_Weapon == nullptr) return;
	
	bRotate_Root_Bone = false;
	float Speed = Calculate_Speed();
	if(Speed > 0.f)
	{
		Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
		return;
	}

	Proxy_Rotation_Last_Frame = Proxy_Rotation;
	Proxy_Rotation = GetActorRotation();
	Proxy_Yaw = UKismetMathLibrary::NormalizedDeltaRotator(Proxy_Rotation, Proxy_Rotation_Last_Frame).Yaw;

	//UE_LOG(LogTemp, Warning, TEXT("Proxy Yaw %f"), Proxy_Yaw);

	if(FMath::Abs(Proxy_Yaw) > Turn_Threshold)
	{	if(Proxy_Yaw > Turn_Threshold)
		{
			Turning_In_Place = ETurning_In_Place::ETIP_Right;
		}
		else if (Proxy_Yaw < -Turn_Threshold)
		{
			Turning_In_Place = ETurning_In_Place::ETIP_Left;
		}
		else
		{
			Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
		}
		return;
	}
	Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
}

bool AShooter_Character::Is_Sprinting()
{
	return (Combat && Combat->bSprinting);
}

void AShooter_Character::Sprint_Button_Pressed()
{
	if(bDisable_Gameplay) return;

	if(Combat && !Combat->bAiming && Combat->Get_Combat_State() != ECombat_State::ECS_Reloading)
	{
		if(bIsCrouched)
		{
			UnCrouch();
		}
		else
		{
			Combat->Sprint_Button_Pressed(true);
		}
	}
}

void AShooter_Character::Sprint_Button_Released()
{
	if(bDisable_Gameplay) return;

	Combat->Sprint_Button_Pressed(false);
}

void AShooter_Character::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}

}

void AShooter_Character::Fire_Button_Pressed()
{
	if(bDisable_Gameplay) return;

	if(Combat && !Combat->bSprinting)
	{
		Combat->Fire_Button_Pressed(true);
	}
}

void AShooter_Character::Fire_Button_Released()
{
	if(bDisable_Gameplay) return;

	if(Combat)
	{
		Combat->Fire_Button_Pressed(false);
	}
}

void AShooter_Character::Turn_In_Place(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("AO_Yaw %f"), AO_Yaw);
	if(AO_Yaw > 90.f)
	{
		Turning_In_Place = ETurning_In_Place::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		Turning_In_Place = ETurning_In_Place::ETIP_Left;
	}
	if(Turning_In_Place != ETurning_In_Place::ETIP_Not_Turning)
	{
		Interp_AO_Yaw = FMath::FInterpTo(Interp_AO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = Interp_AO_Yaw;
		if(FMath::Abs(AO_Yaw) < 15.f)
		{
			Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
			Starting_Aim_Rotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); 
		}
	}
	
}

void AShooter_Character::Hide_Camera_When_Character_Is_Close()
{
	if(!IsLocallyControlled()) return;
	/*if(((Follow_Camera->GetComponentLocation() - GetActorLocation()).Size()) < Camera_Threshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->Equipped_Weapon->Get_Weapon_Mesh())
		{
			Combat->Equipped_Weapon->Get_Weapon_Mesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->Equipped_Weapon->Get_Weapon_Mesh())
		{
			Combat->Equipped_Weapon->Get_Weapon_Mesh()->bOwnerNoSee = false;
		}
	}*/
}

void AShooter_Character::OnRep_Health(float Last_Health)
{
	Update_HUD_Health();
	if(Health < Last_Health)
	{
		Play_Hit_React_Montage();
	}
	
}

void AShooter_Character::OnRep_Armor(float Last_Armor)
{
	Update_HUD_Armor();
	if(Armor < Last_Armor)
	{
		//Play_Hit_React_Montage();
	}
}

void AShooter_Character::Update_HUD_Health()
{
	Shooter_Player_Controller = Shooter_Player_Controller == nullptr ? Cast<AShooter_Player_Controller>(Controller) : Shooter_Player_Controller;
	if(Shooter_Player_Controller)
	{
		Shooter_Player_Controller->Set_HUD_Health(Health, Max_Health);
	}
}

void AShooter_Character::Update_HUD_Armor()
{
	Shooter_Player_Controller = Shooter_Player_Controller == nullptr ? Cast<AShooter_Player_Controller>(Controller) : Shooter_Player_Controller;
	if(Shooter_Player_Controller)
	{
		Shooter_Player_Controller->Set_HUD_Armor(Armor, Max_Armor);
	}
}

void AShooter_Character::Update_HUD_Ammo()
{
	Shooter_Player_Controller = Shooter_Player_Controller == nullptr ? Cast<AShooter_Player_Controller>(Controller) : Shooter_Player_Controller;
	if(Shooter_Player_Controller && Combat && Combat->Equipped_Weapon)
	{
		Shooter_Player_Controller->Set_HUD_Carried_Ammo(Combat->Carried_Ammo);
		Shooter_Player_Controller->Set_HUD_Weapon_Ammo(Combat->Equipped_Weapon->Get_Ammo());
	}
}

void AShooter_Character::Poll_Initialize()
{
	if(Shooter_Player_State == nullptr)
	{
		Shooter_Player_State = GetPlayerState<AShooter_Player_State>();
		if(Shooter_Player_State)
		{
			Shooter_Player_State->Add_To_Score(0.f);
			Shooter_Player_State->Add_To_Deaths(0);
		}
	}
}

void AShooter_Character::Set_Overlapping_Weapon(AWeapon *Weapon)
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

FVector AShooter_Character::Get_Hit_Target() const
{
    if(Combat == nullptr) return FVector();
	return Combat->Hit_Target;
}

ECombat_State AShooter_Character::Get_Combat_State() const
{
     if(Combat == nullptr) return ECombat_State::ECS_MAX;
	return Combat->Combat_State;
}