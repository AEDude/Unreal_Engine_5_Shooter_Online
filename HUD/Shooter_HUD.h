// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Shooter_HUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:

	UPROPERTY()
	class UTexture2D* Crosshair_Center;
	
	UPROPERTY()
	UTexture2D* Crosshair_Top;
	
	UPROPERTY()
	UTexture2D* Crosshair_Bottom;
	
	UPROPERTY()
	UTexture2D* Crosshair_Left;
	
	UPROPERTY()
	UTexture2D* Crosshair_Right;
	
	float Crosshair_Spread;
	FLinearColor Crosshairs_Color;

};

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AShooter_HUD : public AHUD
{
	GENERATED_BODY()

public:

	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> Character_Overlay_Class;
	void Add_Character_Overlay();

	UPROPERTY()
	class UCharacter_Overlay* Character_Overlay;

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<class UUserWidget> Announcement_Class;

	UPROPERTY()
	class UAnnouncement* Announcement;

	void Add_Announcement();

protected:

	virtual void BeginPlay() override;
	
private:

	FHUDPackage HUD_Package;

	void Draw_Crosshair(UTexture2D* Texture, FVector2D Viewport_Center, FVector2D Spread, FLinearColor Crosshair_Color);
	
	UPROPERTY(EditAnywhere)
	float Crosshiar_Spread_Max = 17.f;

public:

	FORCEINLINE void Set_HUD_Package(const FHUDPackage& Package) { HUD_Package = Package; }
	
};
