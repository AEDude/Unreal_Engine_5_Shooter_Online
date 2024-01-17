// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Shooter_Online/Weapon/Weapon_Types.h"
#include "Ammo_Pickup.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AAmmo_Pickup : public APickup
{
	GENERATED_BODY()

protected:

	virtual void On_Sphere_Overlap(
		UPrimitiveComponent* Overlapped_Component,
		AActor* Other_Actor,
		UPrimitiveComponent* Other_Comp,
		int32 Other_Body_Index,
		bool bFrom_Sweep,
		const FHitResult& Sweep_Result
		);

private:

	UPROPERTY(EditDefaultsOnly)
	int32 Ammo_Amount = 30;

	UPROPERTY(EditDefaultsOnly)
	EWeapon_Type Weapon_Type;
};
