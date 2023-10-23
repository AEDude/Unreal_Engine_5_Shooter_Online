// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Player_State.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Shooter_Online/Player_Controller/Shooter_Player_Controller.h"
#include "Net/UnrealNetwork.h"

void AShooter_Player_State::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShooter_Player_State, Deaths);
}

void AShooter_Player_State::Add_To_Score(float Score_Amount)
{
    SetScore(GetScore() + Score_Amount);
    Character = Character == nullptr ? Cast<AShooter_Character>(GetPawn()) : Character;
    if(Character)
    {
        Controller = Controller == nullptr ? Cast<AShooter_Player_Controller>(Character->Controller) : Controller;
        if(Controller)
        {
            Controller->Set_HUD_Score(GetScore());
        }
    } 
}

void AShooter_Player_State::OnRep_Score()
{
    Super::OnRep_Score();
    
    Character = Character == nullptr ? Cast<AShooter_Character>(GetPawn()) : Character;
    if(Character)
    {
        Controller = Controller == nullptr ? Cast<AShooter_Player_Controller>(Character->Controller) : Controller;
        if(Controller)
        {
            Controller->Set_HUD_Score(GetScore());
        }
    }
}

void AShooter_Player_State::Add_To_Deaths(int32 Deaths_Amount)
{
    Deaths += Deaths_Amount;
    Character = Character == nullptr ? Cast<AShooter_Character>(GetPawn()) : Character;
    if(Character)
    {
        Controller = Controller == nullptr ? Cast<AShooter_Player_Controller>(Character->Controller) : Controller;
        if(Controller)
        {
            Controller->Set_HUD_Deaths(Deaths);
        }
    }
}

void AShooter_Player_State::OnRep_Deaths()
{
    Character = Character == nullptr ? Cast<AShooter_Character>(GetPawn()) : Character;
    if(Character)
    {
        Controller = Controller == nullptr ? Cast<AShooter_Player_Controller>(Character->Controller) : Controller;
        if(Controller)
        {
            Controller->Set_HUD_Deaths(Deaths);
        }
    }
}
