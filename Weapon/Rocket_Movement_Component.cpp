// Fill out your copyright notice in the Description page of Project Settings.


#include "Rocket_Movement_Component.h"

URocket_Movement_Component::EHandleBlockingHitResult URocket_Movement_Component::HandleBlockingHit(const FHitResult &Hit, float TimeTick, const FVector &MoveDelta, float &SubTickTimeRemaining)
{
    Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
    return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocket_Movement_Component::HandleImpact(const FHitResult &Hit, float TimeSlice, const FVector &MoveDelta)
{
    //Rocket should not stop, but only explode when their collision detects a hit.
}
