// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Buff_Component.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_ONLINE_API UBuff_Component : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuff_Component();
	friend class AShooter_Character;

	void Heal(float Healing_Amount, float Healing_Time);
	void Replenish_Armor(float Armor_Amount, float Armor_Time);
	void Set_Initial_Speeds(float Base_Speed, float Crouch_Speed);
	void Buff_Speed(float Buff_Base_Speed, float Buff_Crouch_Speed, float Buff_Timer);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void Heal_Ramp_Up(float DeltaTime);
	void Replenish_Armor_Ramp_Up(float DeltaTime);

private:

	UPROPERTY()
	class AShooter_Character* Character;

	/**
	 * Health Buffs
	*/

	bool bHealing = false;
	float Healing_Rate = 0.f;
	float Amount_To_Heal = 0.f;

	/**
	 * Armor Buffs
	*/

	bool bReplenishing_Armor = false;
	float Armor_Replenish_Rate = 0.f;
	float Armor_Replenish_Amount = 0.f;

	/**
	 * Speed Buffs
	*/

	FTimerHandle Speed_Buff_Timer;
	void Reset_Speeds();
	float Initial_Base_Speed = 0.f;
	float Initial_Crouch_Speed = 0.f;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Speed_Buff(float Base_Speed, float Crouch_Speed);


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
