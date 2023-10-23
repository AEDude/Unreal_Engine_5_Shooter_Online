// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Shooter_Online_Game_Mode.generated.h"

namespace MatchState
{
	extern SHOOTER_ONLINE_API const FName Cooldown; // Match duration has been reached; display winner and begin cooldown timer. 
}

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AShooter_Online_Game_Mode : public AGameMode
{
	GENERATED_BODY()

public:

	AShooter_Online_Game_Mode();
	
	virtual void Tick(float DeltaTIme) override;
	
	virtual void Player_Eliminated(class AShooter_Character* Eliminated_Character, class AShooter_Player_Controller* Victim_Controller,  AShooter_Player_Controller* Attacker_Controller);

	virtual void Request_Respawn(ACharacter* Eliminated_Charater, AController* Eliminated_Controller);

	UPROPERTY(EditDefaultsOnly)
	float Warmup_Time = 59.f;

	UPROPERTY(EditDefaultsOnly)
	float Match_Time = 480.f;

	UPROPERTY(EditDefaultsOnly)
	float Cooldown_Time = 10.f;

	float Level_Starting_Time = 0.f;


protected:

	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:

	float Eliminated_Character_Termination_Timer = 7.7f;
	float Countdown_Time = 0.f;

public:
	FORCEINLINE float Get_Countdown_Time() const { return Countdown_Time; }
};
