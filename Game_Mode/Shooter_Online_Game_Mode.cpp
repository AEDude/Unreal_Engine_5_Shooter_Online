// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Online_Game_Mode.h"
#include "Shooter_Online\Character\Shooter_Character.h"
#include "Shooter_Online/Player_Controller/Shooter_Player_Controller.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Shooter_Online/Player_State/Shooter_Player_State.h"
#include "Shooter_Online/Game_State/Shooter_Online_Game_State.h"

namespace MatchState
{
    const FName Cooldown = FName("Cooldown");
}

AShooter_Online_Game_Mode::AShooter_Online_Game_Mode()
{
    bDelayedStart = true;
}

void AShooter_Online_Game_Mode::BeginPlay()
{
    Super::BeginPlay();

    Level_Starting_Time = GetWorld()->GetTimeSeconds();
}

void AShooter_Online_Game_Mode::Tick(float DeltaTIme)
{
    Super::Tick(DeltaTIme);

    if(MatchState == MatchState::WaitingToStart)
    {
        Countdown_Time = Warmup_Time - GetWorld()->GetTimeSeconds() + Level_Starting_Time;
        if(Countdown_Time <= 0.f)
        {
            StartMatch();
        }
    }
    else if(MatchState == MatchState::InProgress)
    {
        Countdown_Time = Warmup_Time + Match_Time - GetWorld()->GetTimeSeconds() + Level_Starting_Time;
        if(Countdown_Time <= 0.f)
        {
            SetMatchState(MatchState::Cooldown);
        }
    }
    else if(MatchState == MatchState::Cooldown)
    {
        Countdown_Time = Cooldown_Time + Warmup_Time + Match_Time - GetWorld()->GetTimeSeconds() + Level_Starting_Time;
        if(Countdown_Time <= 0.f)
        {
            RestartGame();
        }
    }
}

void AShooter_Online_Game_Mode::OnMatchStateSet()
{
    Super::OnMatchStateSet();

    for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AShooter_Player_Controller* Shooter_Player = Cast<AShooter_Player_Controller>(*It);
        if(Shooter_Player)
        {
            Shooter_Player->On_Match_State_Set(MatchState);
        }
    } 
}

void AShooter_Online_Game_Mode::Player_Eliminated(AShooter_Character* Eliminated_Character, AShooter_Player_Controller* Victim_Controller, AShooter_Player_Controller* Attacker_Controller)
{
    AShooter_Player_State* Attacker_Player_State = Attacker_Controller ? Cast<AShooter_Player_State>(Attacker_Controller->PlayerState) : nullptr;
    AShooter_Player_State* Victim_Player_State = Victim_Controller ? Cast<AShooter_Player_State>(Victim_Controller->PlayerState) : nullptr;

    AShooter_Online_Game_State* Shooter_Game_State = GetGameState<AShooter_Online_Game_State>();

    if(Attacker_Player_State && Attacker_Player_State != Victim_Player_State && Shooter_Game_State)
    {
        Attacker_Player_State->Add_To_Score(1.f);
        Shooter_Game_State->Update_Top_Score(Attacker_Player_State);
    }
    if(Victim_Player_State)
    {
        Victim_Player_State->Add_To_Deaths(1);
    }

    if(Eliminated_Character)
    {
        Eliminated_Character->Eliminated();
    }
}

void AShooter_Online_Game_Mode::Request_Respawn(ACharacter *Eliminated_Charater, AController *Eliminated_Controller)
{
    if(Eliminated_Charater)
    {
        Eliminated_Charater->Reset();
        Eliminated_Charater->SetLifeSpan(Eliminated_Character_Termination_Timer);
    }
    if(Eliminated_Controller)
    {
        TArray<AActor*> Player_Starting_Points;
        UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), Player_Starting_Points);
        int32 Selection = FMath::RandRange(0, Player_Starting_Points.Num() -1);
        RestartPlayerAtPlayerStart(Eliminated_Controller, Player_Starting_Points[Selection]);
    }
}