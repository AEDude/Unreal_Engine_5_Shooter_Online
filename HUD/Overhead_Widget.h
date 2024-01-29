// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Overhead_Widget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API UOverhead_Widget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	/*"meta = (BindWidget)" specifier will be initialized on "Display_Text" v
	ariable so that it may be linked to the widget in it's respective child Blueprint.*/

	//Declared for storing the ID of the player.
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Display_Text;

	//Fucntion which sets the text for the "Display_Text" variable. 
	//Includes an input parameter which is used to pass in the text to display.
	void Set_Display_Text(const FString& Text_To_Display);

	//Function which figures out the network role of the character.
	//Includes an input parameter of data type APawn so that it may be called from the character class while
	//passing in a pointer to said character.
	UFUNCTION(BlueprintCallable)
	void Show_Player_Net_Role(APawn* In_Pawn);

	//Function which figures out the ID of the character.
	//Includes an input parameter of data type APawn so that it may be called from the character class while
	//passing in a pointer to said character.
	UFUNCTION(BlueprintCallable)
	void Show_Player_ID(APawn* In_Pawn);

protected:

	//Inherited by UUserWidget. Called when the player transitions to a new level/ leave the current level 
	//and removes the widget from the viewport. 
	virtual void NativeDestruct() override;
	
};
