// Fill out your copyright notice in the Description page of Project Settings.


#include "Bullet_Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
ABullet_Casing::ABullet_Casing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Casing_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Casing Mesh"));
	SetRootComponent(Casing_Mesh);
	Casing_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	Casing_Mesh->SetSimulatePhysics(true);
	Casing_Mesh->SetEnableGravity(true);
	Casing_Mesh->SetNotifyRigidBodyCollision(true);
	Bullet_Shell_Ejection_Impusle = 1.5f;
	Bullet_Casing_Lifespan = 1.f;

}

// Called when the game starts or when spawned
void ABullet_Casing::BeginPlay()
{
	Super::BeginPlay();

	Casing_Mesh->OnComponentHit.AddDynamic(this, &ABullet_Casing::On_Hit);
	Casing_Mesh->AddImpulse(GetActorForwardVector() * Bullet_Shell_Ejection_Impusle);
	SetLifeSpan(Bullet_Casing_Lifespan);
	
}

// Called every frame
void ABullet_Casing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ABullet_Casing::AddActorLocalRotation(Bullet_Casing_Rotation_Velocity * DeltaTime);
}

void ABullet_Casing::On_Hit(UPrimitiveComponent *Hit_Comp, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, FVector Normal_Impulse, const FHitResult &Hit)
{
	if(Bullet_Casing_Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Bullet_Casing_Sound, GetActorLocation());
	}
	Casing_Mesh->SetNotifyRigidBodyCollision(false);
}