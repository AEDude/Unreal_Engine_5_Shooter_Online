// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Player_Controller.h"
#include "Shooter_Online/HUD/Shooter_HUD.h"
#include "Shooter_Online/HUD/Character_Overlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Net/UnrealNetwork.h"
#include "Shooter_Online/Game_Mode/Shooter_Online_Game_Mode.h"
#include "Kismet/GameplayStatics.h"
#include "Shooter_Online/Shooter_Components/Combat_Component.h"
#include "Shooter_Online/Game_State/Shooter_Online_Game_State.h"


void AShooter_Player_Controller::BeginPlay()
{
    Super::BeginPlay();

    Shooter_HUD = Cast<AShooter_HUD>(GetHUD());

    Server_Check_Match_State();
}

void AShooter_Player_Controller::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    Set_HUD_Time();

    Check_Time_Sync(DeltaTime);

    Poll_Initialized();
}

void AShooter_Player_Controller::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AShooter_Player_Controller, Match_State)
}

void AShooter_Player_Controller::Check_Time_Sync(float DeltaTime)
{
    Time_Sync_Running_Time += DeltaTime;
    if(IsLocalController() && Time_Sync_Running_Time > Time_Sync_Frequency)
    {
        Server_Request_Server_Time(GetWorld()->GetTimeSeconds());
        Time_Sync_Running_Time = 0.f;
    }
}

void AShooter_Player_Controller::Poll_Initialized()
{
    if(Character_Overlay == nullptr)
    {
        if(Shooter_HUD && Shooter_HUD->Character_Overlay)
        {
           Character_Overlay = Shooter_HUD->Character_Overlay;
           if(Character_Overlay)
           {
                if(bInitialized_Health) Set_HUD_Health(HUD_Health, HUD_MAX_Health);
                if(bInitialized_Armor) Set_HUD_Armor(HUD_Armor, HUD_Max_Armor);
                if(bInitialized_Score) Set_HUD_Score(HUD_Score);
                if(bInitialized_Deaths) Set_HUD_Deaths(HUD_Deaths);

                AShooter_Character* Shooter_Character = Cast<AShooter_Character>(GetPawn());
                if(Shooter_Character && Shooter_Character->Get_Combat())
                {
                    Set_HUD_Grenades(Shooter_Character->Get_Combat()->Get_Grenades());
                }
                Set_HUD_Grenades(HUD_Grenades);
           }
        }
    }
}

void AShooter_Player_Controller::Server_Check_Match_State_Implementation()
{
    AShooter_Online_Game_Mode* Game_Mode = Cast<AShooter_Online_Game_Mode>(UGameplayStatics::GetGameMode(this));
    if(Game_Mode)
    {
        Warmup_Time = Game_Mode->Warmup_Time;
        Match_Time = Game_Mode->Match_Time;
        Cooldown_Time = Game_Mode->Cooldown_Time;
        Level_Starting_Time = Game_Mode->Level_Starting_Time;
        Match_State = Game_Mode->GetMatchState();
        Client_Join_Mid_Game(Warmup_Time, Match_Time, Cooldown_Time, Level_Starting_Time, Match_State);

        if(Shooter_HUD && Match_State == MatchState::WaitingToStart)
        {
            Shooter_HUD->Add_Announcement();
        }
    }
}

void AShooter_Player_Controller::Client_Join_Mid_Game_Implementation(float Warmup, float Match, float Cooldown, float Starting_Time, FName State_Of_Match)
{
    Warmup_Time = Warmup;
    Match_Time = Match;
    Cooldown_Time = Cooldown;
    Level_Starting_Time = Starting_Time;
    Match_State = State_Of_Match;
    On_Match_State_Set(Match_State);

    if(Shooter_HUD && Match_State == MatchState::WaitingToStart)
    {
        Shooter_HUD->Add_Announcement();
    }
}

void AShooter_Player_Controller::OnPossess(APawn *InPawn)
{
    Super::OnPossess(InPawn);
    
    AShooter_Character* Shooter_Character = Cast<AShooter_Character>(InPawn);
    if(Shooter_Character)
    {
        Set_HUD_Health(Shooter_Character->Get_Health(), Shooter_Character->Get_Max_Health());
        Shooter_Character->Update_HUD_Health();
        Shooter_Character->Update_HUD_Armor();
        Shooter_Character->Update_HUD_Ammo();
    }
}

void AShooter_Player_Controller::Set_HUD_Health(float Health, float Max_Health)
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;
    
    bool bHud_Is_Valid = Shooter_HUD && 
    Shooter_HUD->Character_Overlay && 
    Shooter_HUD->Character_Overlay->Health_Bar && 
    Shooter_HUD->Character_Overlay->Health_Text;
    
    /*if(Shooter_HUD)
    {
        UE_LOG(LogTemp, Warning, TEXT("Shooter HUD is valid."));
    }
    else
    {
         UE_LOG(LogTemp, Warning, TEXT("Shooter HUD is not valid."));
    }
    if(Shooter_HUD && Shooter_HUD->Character_Overlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("Shooter HUD Character Overlay is valid."));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Shooter HUD Character Overlay is not valid."));
    }*/
   
    if(bHud_Is_Valid)
    {
        const float Health_Percent = Health / Max_Health;
        Shooter_HUD->Character_Overlay->Health_Bar->SetPercent(Health_Percent);
        FString Health_Text = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(Max_Health));
        Shooter_HUD->Character_Overlay->Health_Text->SetText(FText::FromString(Health_Text));
    }
    else
    {
        bInitialized_Health = true;
        HUD_Health = Health;
        HUD_MAX_Health = Max_Health;
    }
}

void AShooter_Player_Controller::Set_HUD_Armor(float Armor, float Max_Armor)
{
    Shooter_HUD = Shooter_HUD == nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;

    bool bHud_Is_Valid = Shooter_HUD &&
    Shooter_HUD->Character_Overlay &&
    Shooter_HUD->Character_Overlay->Armor_bar &&
    Shooter_HUD->Character_Overlay->Armor_Text;

    if(bHud_Is_Valid)
    {
        const float Armor_Percent = Armor / Max_Armor;
        Shooter_HUD->Character_Overlay->Armor_bar->SetPercent(Armor_Percent);
        FString Armor_Text = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Armor), FMath::CeilToInt(Max_Armor));
        Shooter_HUD->Character_Overlay->Armor_Text->SetText(FText::FromString(Armor_Text));
    }
    else
    {
        bInitialized_Armor = true;
        HUD_Armor = Armor;
        HUD_Max_Armor = Max_Armor;
    }

}

void AShooter_Player_Controller::Set_HUD_Score(float Score)
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;

    bool bHud_Is_Valid = Shooter_HUD && 
    Shooter_HUD->Character_Overlay && 
    Shooter_HUD->Character_Overlay->Score_Amount;
    if(bHud_Is_Valid)
    {
        FString Score_Text = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
        Shooter_HUD->Character_Overlay->Score_Amount->SetText(FText::FromString(Score_Text));
    }
    else
    {
        bInitialized_Score = true;
        HUD_Score = Score;
    }
}

void AShooter_Player_Controller::Set_HUD_Deaths(int32 Deaths)
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;

    bool bHud_Is_Valid = Shooter_HUD && 
    Shooter_HUD->Character_Overlay && 
    Shooter_HUD->Character_Overlay->Deaths_Amount;
    if(bHud_Is_Valid)
    {
        FString Deaths_Text = FString::Printf(TEXT("%d"), Deaths);
        Shooter_HUD->Character_Overlay->Deaths_Amount->SetText(FText::FromString(Deaths_Text));
    }
    else
    {
        bInitialized_Deaths = true;
        HUD_Deaths = Deaths;
    }
}

void AShooter_Player_Controller::Set_HUD_Weapon_Ammo(int32 Ammo)
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;

    bool bHud_Is_Valid = Shooter_HUD && 
    Shooter_HUD->Character_Overlay && 
    Shooter_HUD->Character_Overlay->Weapon_Ammo_Amount;
    if(bHud_Is_Valid)
    {
        FString Ammo_Text = FString::Printf(TEXT("%d"), Ammo);
        Shooter_HUD->Character_Overlay->Weapon_Ammo_Amount->SetText(FText::FromString(Ammo_Text));
    }
}

void AShooter_Player_Controller::Set_HUD_Carried_Ammo(int32 Ammo)
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;

    bool bHud_Is_Valid = Shooter_HUD && 
    Shooter_HUD->Character_Overlay && 
    Shooter_HUD->Character_Overlay->Carried_Ammo_Amount;
    if(bHud_Is_Valid)
    {
        FString Ammo_Text = FString::Printf(TEXT("%d"), Ammo);
        Shooter_HUD->Character_Overlay->Carried_Ammo_Amount->SetText(FText::FromString(Ammo_Text));
    }
}

void AShooter_Player_Controller::Set_HUD_Match_Countdown(float Countdown_Time)
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;

    bool bHud_Is_Valid = Shooter_HUD && 
    Shooter_HUD->Character_Overlay && 
    Shooter_HUD->Character_Overlay->Match_Countdown_Text;
    if(bHud_Is_Valid)
    {   
        if(Countdown_Time < 0.f)
        {
          Shooter_HUD->Character_Overlay->Match_Countdown_Text->SetText(FText());
          return;  
        }

        int32 Minutes = FMath::FloorToInt(Countdown_Time / 59.5f);
        int32 Seconds = Countdown_Time - Minutes * 59.5;

        FString Countdown_Text = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        Shooter_HUD->Character_Overlay->Match_Countdown_Text->SetText(FText::FromString(Countdown_Text));
    }
}

void AShooter_Player_Controller::Set_HUD_Announcement_Countdown(float Countdown_Time)
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;

    bool bHud_Is_Valid = Shooter_HUD && 
    Shooter_HUD->Announcement && 
    Shooter_HUD->Announcement->Warmup_Time;
    if(bHud_Is_Valid)
    {   
        if(Countdown_Time < 0.f)
        {
          Shooter_HUD->Announcement->Warmup_Time->SetText(FText());
          return;  
        }

        int32 Minutes = FMath::FloorToInt(Countdown_Time / 59.5f);
        int32 Seconds = Countdown_Time - Minutes * 59.5;

        FString Countdown_Text = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
        Shooter_HUD->Announcement->Warmup_Time->SetText(FText::FromString(Countdown_Text));
    }
}

void AShooter_Player_Controller::Set_HUD_Time()
{
    if(HasAuthority())
    {
        AShooter_Online_Game_Mode* Shooter_Game_Mode = Cast<AShooter_Online_Game_Mode>(UGameplayStatics::GetGameMode(this));
        if(Shooter_Game_Mode)
        {
            Level_Starting_Time = Shooter_Game_Mode->Level_Starting_Time;
        }
    }
    
    float Time_Left = 0.f;
    if(Match_State == MatchState::WaitingToStart) Time_Left = Warmup_Time - Get_Server_Time() + Level_Starting_Time;
    else if (Match_State == MatchState::InProgress) Time_Left = Warmup_Time + Match_Time - Get_Server_Time() + Level_Starting_Time;
    else if(Match_State == MatchState::Cooldown) Time_Left = Cooldown_Time + Warmup_Time + Match_Time - Get_Server_Time() + Level_Starting_Time;
   
    uint32 Seconds_Left = FMath::CeilToInt(Time_Left);

    if(HasAuthority())
    {
        Shooter_Online_Game_Mode = Shooter_Online_Game_Mode == nullptr ? Cast<AShooter_Online_Game_Mode>(UGameplayStatics::GetGameMode(this)) : Shooter_Online_Game_Mode;
        if(Shooter_Online_Game_Mode)
        {
            Seconds_Left = FMath::CeilToInt(Shooter_Online_Game_Mode->Get_Countdown_Time() + Level_Starting_Time);
        }
    }

    if(Countdown_Integer != Seconds_Left)
    {
        if(Match_State == MatchState::WaitingToStart || Match_State == MatchState::Cooldown)
        {
            Set_HUD_Announcement_Countdown(Time_Left);
        }
        if(Match_State == MatchState::InProgress)
        {
            Set_HUD_Match_Countdown(Time_Left);
        }
    }

    Countdown_Integer = Seconds_Left;
}

void AShooter_Player_Controller::Set_HUD_Grenades(int32 Grenades)
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;

    bool bHud_Is_Valid = Shooter_HUD && 
    Shooter_HUD->Character_Overlay && 
    Shooter_HUD->Character_Overlay->Grenades_Text;
    if(bHud_Is_Valid)
    {
        FString Grenades_Text = FString::Printf(TEXT("%d"), Grenades);
        Shooter_HUD->Character_Overlay->Grenades_Text->SetText(FText::FromString(Grenades_Text));
    }
    else
    {
        bInitialized_Grenades = true; 
        HUD_Grenades = Grenades; 
    }
}

void AShooter_Player_Controller::Server_Request_Server_Time_Implementation(float Time_Of_Client_Request)
{
    float Server_Time_Of_Receipt = GetWorld()->GetTimeSeconds();
    Client_Report_Server_Time(Time_Of_Client_Request, Server_Time_Of_Receipt);
}

void AShooter_Player_Controller::Client_Report_Server_Time_Implementation(float Time_Of_Client_Request, float Time_Server_Received_Client_Request)
{
    float Round_Trip_Time = GetWorld()->GetTimeSeconds() - Time_Of_Client_Request;
    float Current_Server_Time = Time_Server_Received_Client_Request + (0.5f * Round_Trip_Time);
    Client_Server_Delta = Current_Server_Time - GetWorld()->GetTimeSeconds();
}

float AShooter_Player_Controller::Get_Server_Time()
{
    if(HasAuthority()) 
    {
        return GetWorld()->GetTimeSeconds();
    }
    else
    {
        return GetWorld()->GetTimeSeconds() + Client_Server_Delta;
    } 
}

void AShooter_Player_Controller::ReceivedPlayer()
{
    Super::ReceivedPlayer();

    if(IsLocalController())
    {
        Server_Request_Server_Time(GetWorld()->GetTimeSeconds());
    }
}

void AShooter_Player_Controller::On_Match_State_Set(FName State)
{
    Match_State = State;

    if(Match_State == MatchState::InProgress)
    {
         Handle_Match_Started();
    }
    else if(Match_State == MatchState::Cooldown)
    {
        Handle_Cooldown();
    }
}

void AShooter_Player_Controller::OnRep_Match_State()
{
    if(Match_State == MatchState::InProgress)
    {
         Handle_Match_Started();
    }
    else if(Match_State == MatchState::Cooldown)
    {
        Handle_Cooldown();
    }
}

void AShooter_Player_Controller::Handle_Match_Started()
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;
         if(Shooter_HUD)
         {
            Shooter_HUD->Add_Character_Overlay();
            if(Shooter_HUD->Announcement)
            {
                Shooter_HUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
            }
         }
}

void AShooter_Player_Controller::Handle_Cooldown()
{
    Shooter_HUD = Shooter_HUD = nullptr ? Cast<AShooter_HUD>(GetHUD()) : Shooter_HUD;
    if(Shooter_HUD)
    {
        Shooter_HUD->Character_Overlay->RemoveFromParent();
        bool bHUD_Is_Valid = Shooter_HUD->Announcement && 
        Shooter_HUD->Announcement->Announcement_Text && 
        Shooter_HUD->Announcement->Information_Text;
            
        if(bHUD_Is_Valid)
        {
            Shooter_HUD->Announcement->SetVisibility(ESlateVisibility::Visible);
            FString Announcement_Text("NEW MATCH STARTS IN:");
            Shooter_HUD->Announcement->Announcement_Text->SetText(FText::FromString(Announcement_Text));

            AShooter_Online_Game_State* Shooter_Game_State = Cast<AShooter_Online_Game_State>(UGameplayStatics::GetGameState(this));
            AShooter_Player_State* Shooter_Player_State = GetPlayerState<AShooter_Player_State>();
            if(Shooter_Game_State && Shooter_Player_State)
            {
                TArray<AShooter_Player_State*> Top_Players = Shooter_Game_State->Top_Scoring_Players;
                FString Information_Text_String;
                if(Top_Players.Num() == 0)
                {
                    Information_Text_String = FString("THERE IS NO WINNER...");
                }
                else if (Top_Players.Num() == 1 && Top_Players[0] == Shooter_Player_State)
                {
                    Information_Text_String = FString("YOU ARE VICTORIOUS!");
                }
                else if(Top_Players.Num() == 1)
                {
                    Information_Text_String = FString::Printf(TEXT("WINNER: \n %s"), *Top_Players[0]->GetPlayerName());
                }
                else if(Top_Players.Num() > 1)
                {
                    Information_Text_String = FString("PLAYERS TIED FOR THE WIN: \n");
                    for(auto TiedPlayer : Top_Players)
                    {
                        Information_Text_String.Append(FString::Printf(TEXT("%s \n"), *TiedPlayer->GetPlayerName()));
                    }
                }

                Shooter_HUD->Announcement->Information_Text->SetText(FText::FromString(Information_Text_String));
            }
        }
    }
    AShooter_Character* Shooter_Character = Cast<AShooter_Character>(GetPawn());
    if(Shooter_Character && Shooter_Character->Get_Combat())
    {
        Shooter_Character->bDisable_Gameplay = true;
        Shooter_Character->Get_Combat()->Fire_Button_Pressed(false);
    }
}
