// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "Shooter_Online_Game_State.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AShooter_Online_Game_State : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	void Update_Top_Score(class AShooter_Player_State* Scoring_Player);

	UPROPERTY(Replicated)
	TArray<class AShooter_Player_State*> Top_Scoring_Players;

private:

	float Top_Score = 0.f;



};
