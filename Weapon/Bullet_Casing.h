// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Bullet_Casing.generated.h"

UCLASS()
class SHOOTER_ONLINE_API ABullet_Casing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABullet_Casing();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void On_Hit(UPrimitiveComponent* Hit_Comp, AActor* Other_Actor, UPrimitiveComponent* Other_Comp, FVector Normal_Impulse, const FHitResult& Hit);

private:
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Casing_Mesh;

	UPROPERTY(EditAnywhere)
	float Bullet_Shell_Ejection_Impusle;

	UPROPERTY(EditAnywhere)
	class USoundCue* Bullet_Casing_Sound;

	FRotator Bullet_Casing_Rotation_Velocity;

	float Bullet_Casing_Lifespan;
};
