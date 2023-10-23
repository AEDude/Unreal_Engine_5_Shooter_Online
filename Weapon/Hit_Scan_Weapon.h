// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Hit_Scan_Weapon.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AHit_Scan_Weapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& Hit_Target) override;


protected:

	FVector Trace_End_With_Scatter(const FVector& Trace_Start, const FVector& Hit_Target);
	void Weapon_Trace_Hit(const FVector& Trace_Start, const FVector& Hit_Target, FHitResult& Out_Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 15.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Impact_Particles;

	UPROPERTY(EditAnywhere)
	USoundCue* Hit_Sound;

private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* Beam_Particles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Muzzle_Flash;

	UPROPERTY(EditAnywhere)
	USoundCue* Fire_Sound;

	/**
	 * Trace End with scatter
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float Distance_To_Sphere = 777.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float Sphere_Radius = 70.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUse_Weapon_Scatter = false;
};
