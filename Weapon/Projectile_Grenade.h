// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Projectile_Grenade.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AProjectile_Grenade : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectile_Grenade();
	virtual void Destroyed() override;

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void On_Bounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:

	UPROPERTY(EditAnywhere)
	USoundCue* Bounce_Sound;

	
};
