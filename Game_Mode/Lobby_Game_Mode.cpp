// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby_Game_Mode.h"
#include "GameFramework/GameStateBase.h"

void ALobby_Game_Mode::PostLogin(APlayerController* New_Player)
{
    //Call "Super" when overriding a function
    Super::PostLogin(New_Player);

    //Accessing the AGameStateBase which is stored by the TObjectPtr GameState using "Get()".
    //This is to get the amount of players in the game.
    int32 Number_Of_Players{GameState.Get()->PlayerArray.Num()};
    
    //Defining how many players are needed in the lobby in order to initiate Seamless Travel out of the lobby 
    //and into the level which will be played.
    if(Number_Of_Players == 2)
    {
        //Initializing a UWorld pointer so "ServerTravel" may be called from the UWorld class.
        UWorld* World{GetWorld()};
        if(World)
        {
            //Setting SeamlessTravel to true (It is set to true by default, but just to be safe it is done here as well and also in the Lobby_Game_Mode BP).
            //Needs a transition map to be set in project settings.
            bUseSeamlessTravel = true;
            
            //ServerTravel is called by using the UWorld pointer. ServerTravel defines which map to travel to.
                                                                    //The "?listen" in the map address specifies that this map will be loaded as a Listen Server.
            World->ServerTravel(FString("/Game/Maps/Shooter_UADistrict_01_P?listen"));
        }
    }
}

//The GameMode only exists on the server, therefore only the server has authority to access anything in the GameMode class.
//TObjectPtr GameState holds AGameStateBase which has an array of the player states which joined the game.
/*PlayerArray is a TArray data type which means it has the "Num()" function. This fucntion returns the amount of elements 
in said array via an int32 data type.*/