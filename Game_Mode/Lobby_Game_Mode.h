// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Lobby_Game_Mode.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API ALobby_Game_Mode : public AGameMode
{
	GENERATED_BODY()
public:
	
	virtual void PostLogin(APlayerController* New_Player) override;
	
};
