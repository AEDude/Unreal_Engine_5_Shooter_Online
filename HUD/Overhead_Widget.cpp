// Fill out your copyright notice in the Description page of Project Settings.


#include "Overhead_Widget.h"
#include "Components/TextBlock.h"

void UOverhead_Widget::Set_Display_Text(const FString& Text_To_Display)
{
    //Checking to see if the pointer "Display_Text" doesn't point to a null ptr.
    if(Display_Text)
    {
        //Convert from FString to FText in order to call the function "SetText()".
        Display_Text->SetText(FText::FromString(Text_To_Display));
    }
}

void UOverhead_Widget::Show_Player_Net_Role(APawn* In_Pawn)
{
    //Obtaining the remote role of the input parameter "In_Pawn" (pointer to the character).
    //A players role is an ENUM value and that ENUM is "ENetRole".
    ENetRole Remote_Role{In_Pawn->GetRemoteRole()};
    
    //Declaring a local FString variable named "Role" to store the net role of the current character.
    //The string stored in this variable will be displayed on the "Display_Text" widget.
    FString Role{};
    
    switch (Remote_Role)
    {
        //ROLE_Authority is for the server
        case ENetRole::ROLE_Authority:
        Role = FString("Authority");
        break;
        
        //ROLE_AutonomousProxy is for a player that is being controlled on this computer.
        case ENetRole::ROLE_AutonomousProxy:
        Role = FString("Autonomous_Proxy");
        break;

        //ROLE_SimulatedProxy is for a player that is being controlled on another computer.
        case ENetRole::ROLE_SimulatedProxy:
        Role = FString("Simulated_Proxy");
        break;
        
        //Won't be applicable in this game, howver an example is an AI.
        case ENetRole::ROLE_None:
        Role = FString("None");
        break;
    }

    //Format a the string to display on the overhead widget bound to this class.
    FString Remote_Role_String = FString::Printf(TEXT("Remote Role %s"), *Role);
    
    //Calling "Set_Display_Text" and passing in "Remote_Role_String" so that the role of the character 
    //can be displayed on the overhead widget bound to this class.
    Set_Display_Text(Remote_Role_String);
}

void UOverhead_Widget::Show_Player_ID(APawn *In_Pawn)
{
}

void UOverhead_Widget::NativeDestruct()
{   
    //Removes widget from the viewport.
    RemoveFromParent();

    //"Super" calls the original version of the function before it is overriden.
    Super::NativeDestruct();
}
