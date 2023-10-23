// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Warmup_Time;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Announcement_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Information_Text;
	
};
