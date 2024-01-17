// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class SHOOTER_ONLINE_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickup();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void Destroyed() override;

protected:

	UFUNCTION()
	virtual void On_Sphere_Overlap(
		UPrimitiveComponent* Overlapped_Component,
		AActor* Other_Actor,
		UPrimitiveComponent* Other_Comp,
		int32 Other_Body_Index,
		bool bFrom_Sweep,
		const FHitResult& Sweep_Result
		);

	UPROPERTY(EditDefaultsOnly)
	float Base_Turn_Rate = 45.f;

	UPROPERTY(EditDefaultsOnly)
	FVector Raise_Lower_Velocity = FVector(0, 0, 45);

	UPROPERTY(EditDefaultsOnly)
	float Maximum_Move_Distance = 40;

	FVector Starting_Location = FVector(0, 0, 0);

	void Raise_Lower_Pickup(float DeltaTime);

	void Raise_Pickup(float DeltaTime);

	void Lower_Pickup(float DeltatTime);

	float Distance_Moved() const;

	bool Should_Pickup_Return() const;



private:
	
	UPROPERTY(EditDefaultsOnly)
	class USphereComponent* Overlap_Sphere;

	UPROPERTY(EditDefaultsOnly)
	class USoundCue* Pickup_Sound;

	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* Pickup_Mesh;
public:	
	

};
