// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Overhead_Widget.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API UOverhead_Widget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Display_Text;

	void Set_Display_Text(FString Text_To_Display);

	UFUNCTION(BlueprintCallable)
	void Show_Player_Net_Role(APawn* In_Pawn);

protected:
	virtual void NativeDestruct() override;
	
};
