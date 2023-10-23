// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h" 
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h" 
#include "Bullet_Casing.h"
#include "Shooter_Online/Player_Controller/Shooter_Player_Controller.h"
// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);

	Weapon_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon_Mesh"));
	SetRootComponent(Weapon_Mesh);

	Weapon_Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Area_Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area_Sphere"));
	Area_Sphere->SetupAttachment(RootComponent);
	Area_Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Area_Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Pickup_Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup_Widget"));
	Pickup_Widget->SetupAttachment(RootComponent);
	

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		Area_Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Area_Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		Area_Sphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::On_Sphere_Overlap);
		Area_Sphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::On_Sphere_End_Overlap);
	}
	if(Pickup_Widget)
	{
		Pickup_Widget->SetVisibility(false);
	}
	
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, Weapon_State);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::On_Sphere_Overlap(UPrimitiveComponent *Overlapped_Component, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, int32 Other_Body_Index, bool bFrom_Sweep, const FHitResult &Sweep_Result)
{
	AShooter_Character* Shooter_Character = Cast<AShooter_Character>(Other_Actor);
	if(Shooter_Character)
	{
		Shooter_Character->Set_Overlapping_Weapon(this);
	}
}

void AWeapon::On_Sphere_End_Overlap(UPrimitiveComponent *Overlapped_Component, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, int32 Other_Body_Index)
{
	AShooter_Character* Shooter_Character = Cast<AShooter_Character>(Other_Actor);
	if(Shooter_Character)
	{
		Shooter_Character->Set_Overlapping_Weapon(nullptr);
	}
}

void AWeapon::Set_HUD_Ammo()
{
	Shooter_Owner_Character = Shooter_Owner_Character == nullptr ? Cast<AShooter_Character>(GetOwner()) : Shooter_Owner_Character;
	if(Shooter_Owner_Character)
	{
		Shooter_Owner_Controller = Shooter_Owner_Controller == nullptr ? Cast<AShooter_Player_Controller>(Shooter_Owner_Character->Controller) : Shooter_Owner_Controller;
		if(Shooter_Owner_Controller)
		{
			Shooter_Owner_Controller->Set_HUD_Weapon_Ammo(Ammo);
		}
	}
}

void AWeapon::Spend_Round()
{
	Ammo = FMath::Clamp(Ammo -1, 0, Magazine_Capacity);
	Set_HUD_Ammo();
}

void AWeapon::OnRep_Ammo()
{
	Shooter_Owner_Character = Shooter_Owner_Character == nullptr ? Cast<AShooter_Character>(GetOwner()) : Shooter_Owner_Character;
	Set_HUD_Ammo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)
	{	
		Shooter_Owner_Character = nullptr;
		Shooter_Owner_Controller = nullptr;
	}
	else
	{
		Set_HUD_Ammo();
	}
}

void AWeapon::Set_Weapon_State(EWeaponState State)
{
	Weapon_State = State;
	switch (Weapon_State)
	{
	case EWeaponState::EWS_Equipped:
		Show_Pickup_Widget(false);
		Area_Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Weapon_Mesh->SetSimulatePhysics(false);
		Weapon_Mesh->SetEnableGravity(false);
		Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;

	case EWeaponState::EWS_Dropped:
		if(HasAuthority())
		{
			Area_Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		Weapon_Mesh->SetSimulatePhysics(true);
		Weapon_Mesh->SetEnableGravity(true);
		Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;
	}
}

void AWeapon::OnRep_Weapon_State()
{
	switch (Weapon_State)
	{
	case EWeaponState::EWS_Equipped:
		Show_Pickup_Widget(false);
		Weapon_Mesh->SetSimulatePhysics(false);
		Weapon_Mesh->SetEnableGravity(false);
		Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;

		case EWeaponState::EWS_Dropped:
		Weapon_Mesh->SetSimulatePhysics(true);
		Weapon_Mesh->SetEnableGravity(true);
		Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;
	}
}

void AWeapon::Show_Pickup_Widget(bool bShow_Widet)
{
	if(Pickup_Widget)
	{
		Pickup_Widget->SetVisibility(bShow_Widet);
	}
}

void AWeapon::Fire(const FVector& Hit_Target)
{
	if(Fire_Animation)
	{
		Weapon_Mesh->PlayAnimation(Fire_Animation, false);
	}
	if(Bullet_Casing_Class)
	{
		const USkeletalMeshSocket* Ammo_Eject_Socket = Weapon_Mesh->GetSocketByName(FName("AmmoEject"));
    	if(Ammo_Eject_Socket)
    	{
        	FTransform Socket_Transform = Ammo_Eject_Socket->GetSocketTransform(Weapon_Mesh);

    		UWorld* World = GetWorld();
        	if(World)
        	{
            	World->SpawnActor<ABullet_Casing>(
            	Bullet_Casing_Class,
            	Socket_Transform.GetLocation(),
            	Socket_Transform.GetRotation().Rotator()
            	);
        	}
    	}
	}
	Spend_Round();
}

void AWeapon::Drop_Weapons()
{
	Set_Weapon_State(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules Detach_Rules(EDetachmentRule::KeepWorld, true);
	Weapon_Mesh->DetachFromComponent(Detach_Rules);
	SetOwner(nullptr);
	Shooter_Owner_Character = nullptr;
	Shooter_Owner_Controller = nullptr;

}

void AWeapon::Add_Ammo(int32 Ammo_To_Add)
{
	Ammo = FMath::Clamp(Ammo - Ammo_To_Add, 0, Magazine_Capacity);
	Set_HUD_Ammo();
}

bool AWeapon::Is_Empty()
{
    return Ammo <= 0;
}