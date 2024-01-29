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
	
	//Keeps track of how many players enter the lobby. This will be used to determine when to notify the 
	//server to get a hold of all the player controllers in the lobby before calling ServerTravel.
	//First function that is safe to access the player controller which moct recently joined.
	virtual void PostLogin(APlayerController* New_Player) override;
	
};
