// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat_Component.h"
#include "Shooter_Online/Weapon/Weapon.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UCombat_Component::UCombat_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	
	PrimaryComponentTick.bCanEverTick = false;

	Base_Walk_Speed = 450.f;
	Aim_Walk_Speed = 300.f;

	// ...
}


// Called when the game starts
void UCombat_Component::BeginPlay()
{
	Super::BeginPlay();

	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Base_Walk_Speed;
	}
	// ...
	
}

void UCombat_Component::Set_Aiming(bool bIs_Aiming)
{
	bAiming = bIs_Aiming;
	Server_Set_Aiming(bIs_Aiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIs_Aiming ? Aim_Walk_Speed : Base_Walk_Speed;
	}
}

void UCombat_Component::Server_Set_Aiming_Implementation(bool bIs_Aiming)
{
	bAiming = bIs_Aiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIs_Aiming ? Aim_Walk_Speed : Base_Walk_Speed;
	}
}

void UCombat_Component::OnRep_Equipped_Weapon()
{
	if(Equipped_Weapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

// Called every frame
void UCombat_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCombat_Component::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCombat_Component, Equipped_Weapon);
	DOREPLIFETIME(UCombat_Component, bAiming);
}

void UCombat_Component::Equip_Weapon(AWeapon* Weapon_To_Equip)
{
	if(Character == nullptr || Weapon_To_Equip == nullptr) return;
	
	Equipped_Weapon = Weapon_To_Equip;
	Equipped_Weapon->Set_Weapon_State(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* Hand_Socket = Character->GetMesh()->GetSocketByName(FName("Right_Hand_Socket"));
	if(Hand_Socket)
	{
		Hand_Socket->AttachActor(Equipped_Weapon, Character->GetMesh());
	}
	
	Equipped_Weapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}
