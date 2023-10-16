// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class SHOOTER_ONLINE_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void On_Hit(UPrimitiveComponent* Hit_Comp, AActor* Other_Actor, UPrimitiveComponent* Other_Comp, FVector Normal_Impulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_On_Hit(bool bCharacter_Hit);

private:

	UPROPERTY(EditAnywhere)
	class UBoxComponent* Collision_Box;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* Projectile_Movement_Component;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	class UParticleSystemComponent* Tracer_Component;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Impact_Particles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Impact_Obstacle_Particles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Impact_Character_Particles;

	UPROPERTY(EditAnywhere)
	class USoundCue* Impact_Sound;

public:	


};
