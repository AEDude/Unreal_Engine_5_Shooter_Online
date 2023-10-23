// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Rocket_Movement_Component.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API URocket_Movement_Component : public UProjectileMovementComponent
{
	GENERATED_BODY()

protected: 

	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice=0.f, const FVector& MoveDelta = FVector::ZeroVector) override;

	
};
