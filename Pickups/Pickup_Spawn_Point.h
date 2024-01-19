// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup_Spawn_Point.generated.h"

UCLASS()
class SHOOTER_ONLINE_API APickup_Spawn_Point : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup_Spawn_Point();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:

	UPROPERTY(EditDefaultsOnly)
	TArray<TSubclassOf<class APickup>> Pickup_Classes;

	UPROPERTY()
	APickup* Spawned_Pickup;

	void Spawn_Pickup();
	
	UFUNCTION()
	void Start_Spawn_Pikup_Timer(AActor* Destroyed_Actor);
	
	void Spawn_Pickup_Timer_Finished();

private:

	FTimerHandle Spawn_Pickup_Timer;

	UPROPERTY(EditDefaultsOnly)
	float Spawn_Pickup_Timer_Minimum = 0.f;
	
	UPROPERTY(EditDefaultsOnly)
	float Spawn_Pickup_Timer_Maximum = 0.f;

public:	
	

};
