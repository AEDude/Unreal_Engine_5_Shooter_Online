// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Projectile_Bullet.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AProjectile_Bullet : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectile_Bullet();

protected:

	virtual void On_Hit(UPrimitiveComponent* Hit_Comp, AActor* Other_Actor, UPrimitiveComponent* Other_Comp, FVector Normal_Impulse, const FHitResult& Hit) override;
};
