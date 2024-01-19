// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Armor_Pickup.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AArmor_Pickup : public APickup
{
	GENERATED_BODY()

public:

	AArmor_Pickup();

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
	float Armor_Replenish_Amount = 100.f;
	
	UPROPERTY(EditDefaultsOnly)
	float Replenish_Armor_Time = 3.f;
	
};
