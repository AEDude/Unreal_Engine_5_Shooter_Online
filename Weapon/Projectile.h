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

	void Start_Destroy_Timer();

	void Destroy_Timer_Finished();

	void Spawn_Trail_System();

	void Explode_Damage();

	UFUNCTION()
	virtual void On_Hit(UPrimitiveComponent* Hit_Comp, AActor* Other_Actor, UPrimitiveComponent* Other_Comp, FVector Normal_Impulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_On_Hit(bool bCharacter_Hit);

	
	UPROPERTY(EditAnywhere)
	float Damage = 15;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* Collision_Box;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Impact_Particles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Impact_Obstacle_Particles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* Impact_Character_Particles;

	UPROPERTY(EditAnywhere)
	class USoundCue* Impact_Sound;

	UPROPERTY(EditAnywhere)
	float Damage_Inner_Radius = 333.f;

	UPROPERTY(EditAnywhere)
	float Damage_Outer_Radius = 555.f;
	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* Trail_System;

	UPROPERTY()
	class UNiagaraComponent* Trail_System_Component;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* Projectile_Movement_Component;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Projectile_Mesh;


private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* Tracer_Component;

	FTimerHandle Destroy_Timer;

	UPROPERTY(EditAnywhere)
	float Destroy_Time = 3.f;


public:	


};
