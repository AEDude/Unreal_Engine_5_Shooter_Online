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

	class UTexture2D* Crosshair_Center;
	UTexture2D* Crosshair_Top;
	UTexture2D* Crosshair_Bottom;
	UTexture2D* Crosshair_Left;
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

private:

	FHUDPackage HUD_Package;

	void Draw_Crosshair(UTexture2D* Texture, FVector2D Viewport_Center, FVector2D Spread, FLinearColor Crosshair_Color);
	
	UPROPERTY(EditAnywhere)
	float Crosshiar_Spread_Max = 17.f;

public:

	FORCEINLINE void Set_HUD_Package(const FHUDPackage& Package) { HUD_Package = Package; }
	
};
