// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character_Overlay.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API UCharacter_Overlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* Armor_bar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Armor_Text;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* Health_Bar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Health_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Score_Amount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Deaths_Amount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Weapon_Ammo_Amount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Carried_Ammo_Amount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Match_Countdown_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Grenades_Text;

	UPROPERTY(meta = (BindWidget))
	class UImage* Packet_Image;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* Packet_Animation;
};
