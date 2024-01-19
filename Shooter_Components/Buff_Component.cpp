// Fill out your copyright notice in the Description page of Project Settings.


#include "Buff_Component.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values for this component's properties
UBuff_Component::UBuff_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UBuff_Component::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UBuff_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Heal_Ramp_Up(DeltaTime);

	Replenish_Armor_Ramp_Up(DeltaTime);

	// ...
}

void UBuff_Component::Heal_Ramp_Up(float DeltaTime)
{
	if(!bHealing || Character == nullptr || Character->Is_Eliminated()) return;

	const float Heal_This_Frame = Healing_Rate * DeltaTime;
	Character->Set_Health(FMath::Clamp(Character->Get_Health() + Heal_This_Frame, 0.f, Character->Get_Max_Health()));
	Character->Update_HUD_Health();
	Amount_To_Heal -= Heal_This_Frame;

	if(Amount_To_Heal <= 0.f || Character->Get_Health() >= Character->Get_Max_Health())
	{
		bHealing = false;
		Amount_To_Heal = 0.f;
	}

}

void UBuff_Component::Replenish_Armor_Ramp_Up(float DeltaTime)
{
	if(!bReplenishing_Armor || Character == nullptr || Character->Is_Eliminated()) return;

	const float Replenish_Armor_This_Frame = Armor_Replenish_Rate * DeltaTime;
	Character->Set_Armor(FMath::Clamp(Character->Get_Armor() + Replenish_Armor_This_Frame, 0.f, Character->Get_Max_Armor()));
	Character->Update_HUD_Armor();
	Armor_Replenish_Amount -= Replenish_Armor_This_Frame;

	if(Armor_Replenish_Amount <= 0.f || Character->Get_Armor() >= Character->Get_Max_Armor())
	{
		bReplenishing_Armor = false;
		Armor_Replenish_Amount = 0.f;
	}

}

void UBuff_Component::Heal(float Healing_Amount, float Healing_Time)
{
	bHealing = true;
	Healing_Rate = Healing_Amount / Healing_Time;
	Amount_To_Heal += Healing_Amount;
}

void UBuff_Component::Replenish_Armor(float Armor_Amount, float Armor_Time)
{
	bReplenishing_Armor = true;
	Armor_Replenish_Rate = Armor_Amount / Armor_Time;
	Armor_Replenish_Amount += Armor_Amount;
}

void UBuff_Component::Set_Initial_Speeds(float Base_Speed, float Crouch_Speed)
{
	Initial_Base_Speed = Base_Speed;
	Initial_Crouch_Speed = Crouch_Speed;
}

void UBuff_Component::Buff_Speed(float Buff_Base_Speed, float Buff_Crouch_Speed, float Buff_Timer)
{
	if(Character == nullptr) return;
	
	Character->GetWorldTimerManager().SetTimer(
		Speed_Buff_Timer,
		this,
		&UBuff_Component::Reset_Speeds,
		Buff_Timer
	);

	if(Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Buff_Base_Speed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Buff_Crouch_Speed;
	}
	Multicast_Speed_Buff(Buff_Base_Speed, Buff_Crouch_Speed);
}

void UBuff_Component::Reset_Speeds()
{
	if(Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Initial_Base_Speed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Initial_Crouch_Speed;
		Multicast_Speed_Buff(Initial_Base_Speed, Initial_Crouch_Speed);
	}
}

void UBuff_Component::Multicast_Speed_Buff_Implementation(float Base_Speed, float Crouch_Speed)
{
	Character->GetCharacterMovement()->MaxWalkSpeed = Base_Speed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = Crouch_Speed;
}
