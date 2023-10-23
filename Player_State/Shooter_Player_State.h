// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Shooter_Player_State.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AShooter_Player_State : public APlayerState
{
	GENERATED_BODY()

public:
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
	/**
	 * Replication Notifies
	*/
	virtual void OnRep_Score() override;
	
	UFUNCTION()
	virtual void OnRep_Deaths();
	
	void Add_To_Score(float Score_Amount);
	void Add_To_Deaths(int32 Deaths_Amount);

private:
	UPROPERTY()
	class AShooter_Character* Character;
	UPROPERTY()
	class AShooter_Player_Controller* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;
	
};
