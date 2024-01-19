// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Health_Pickup.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AHealth_Pickup : public APickup
{
	GENERATED_BODY()

public:

	AHealth_Pickup();

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
	float Healing_Amount = 100.f;
	
	UPROPERTY(EditDefaultsOnly)
	float Healing_Time = 7.f;
};
