// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Speed_Pickup.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API ASpeed_Pickup : public APickup
{
	GENERATED_BODY()
public:

	ASpeed_Pickup();


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
	float Base_Speed_Buff = 1700.f;

	UPROPERTY(EditDefaultsOnly)
	float Crouch_Speed_Buff = 850.f;

	UPROPERTY(EditDefaultsOnly)
	float Speed_Buff_Timer = 30.f;
};
