// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Projectile_Rocket.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AProjectile_Rocket : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectile_Rocket();
	virtual void Destroyed() override;




protected:

	virtual void On_Hit(UPrimitiveComponent* Hit_Comp, AActor* Other_Actor, UPrimitiveComponent* Other_Comp, FVector Normal_Impulse, const FHitResult& Hit) override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	USoundCue* Projectile_Loop;

	UPROPERTY()
	UAudioComponent* Projectile_Loop_Component;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* Looping_Sound_Attenuation;

	UPROPERTY(VisibleAnywhere)
	class URocket_Movement_Component* Rocket_Movement_Component;


private:
};
