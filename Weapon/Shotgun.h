// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Hit_Scan_Weapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AShotgun : public AHit_Scan_Weapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& Hit_Target) override;


private:

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 Number_Of_Shot = 33;
	
};
