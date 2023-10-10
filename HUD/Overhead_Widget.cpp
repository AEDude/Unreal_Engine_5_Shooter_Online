// Fill out your copyright notice in the Description page of Project Settings.


#include "Overhead_Widget.h"
#include "Components/TextBlock.h"

void UOverhead_Widget::Set_Display_Text(FString Text_To_Display)
{
    if(Display_Text)
    {
        Display_Text->SetText(FText::FromString(Text_To_Display));
    }
}

void UOverhead_Widget::Show_Player_Net_Role(APawn *In_Pawn)
{
    ENetRole Remote_Role = In_Pawn->GetRemoteRole();
    FString Role;
    switch (Remote_Role)
    {
        case ENetRole::ROLE_Authority:
        Role = FString("Authority");
        break;

        case ENetRole::ROLE_AutonomousProxy:
        Role = FString("Autonomous_Proxy");
        break;

        case ENetRole::ROLE_SimulatedProxy:
        Role = FString("Simulated_Proxy");
        break;

        case ENetRole::ROLE_None:
        Role = FString("None");
        break;
    }
    FString Remote_Role_String = FString::Printf(TEXT("Remote Role %s"), *Role);
    Set_Display_Text(Remote_Role_String);
}

void UOverhead_Widget::NativeDestruct()
{
    RemoveFromParent();
    Super::NativeDestruct();
}
