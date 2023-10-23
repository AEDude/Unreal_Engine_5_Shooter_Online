// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Online_Game_State.h"
#include "Net/UnrealNetwork.h"
#include "Shooter_Online/Player_State/Shooter_Player_State.h"


void AShooter_Online_Game_State::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShooter_Online_Game_State, Top_Scoring_Players);
}

void AShooter_Online_Game_State::Update_Top_Score(AShooter_Player_State* Scoring_Player)
{
    if(Top_Scoring_Players.Num() == 0)
    {
        Top_Scoring_Players.Add(Scoring_Player);
        Top_Score = Scoring_Player->GetScore();
    }
    else if (Scoring_Player->GetScore() == Top_Score)
    {
        Top_Scoring_Players.AddUnique(Scoring_Player);
    }
    else if (Scoring_Player->GetScore() > Top_Score)
    {
        Top_Scoring_Players.Empty();
        Top_Scoring_Players.AddUnique(Scoring_Player);
        Top_Score = Scoring_Player->GetScore();
    }
    
    
}
