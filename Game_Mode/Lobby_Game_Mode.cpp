// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby_Game_Mode.h"
#include "GameFramework/GameStateBase.h"

void ALobby_Game_Mode::PostLogin(APlayerController* New_Player)
{
    Super::PostLogin(New_Player);

    int32 Number_Of_Players = GameState.Get()->PlayerArray.Num();
    if(Number_Of_Players == 2)
    {
        UWorld* World = GetWorld();
        {
            bUseSeamlessTravel = true;
            World->ServerTravel(FString("/Game/Maps/Shooter_UADistrict_01_P?listen"));
        }
        
    }
}
