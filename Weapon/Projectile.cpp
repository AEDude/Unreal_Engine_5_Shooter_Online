// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Shooter_Online\Character\Shooter_Character.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Collision_Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	SetRootComponent(Collision_Box);
	Collision_Box->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	Collision_Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Collision_Box->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Collision_Box->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	Collision_Box->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

	Projectile_Movement_Component = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	Projectile_Movement_Component->bRotationFollowsVelocity = true;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	if(Tracer)
	{
		Tracer_Component = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			Collision_Box,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if(HasAuthority())
	{
		Collision_Box->OnComponentHit.AddDynamic(this, &AProjectile::On_Hit);
	}
}

void AProjectile::On_Hit(UPrimitiveComponent *Hit_Comp, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, FVector Normal_Impulse, const FHitResult &Hit)
{
	Destroy();
	
	/*AShooter_Character* Shooter_Character = Cast<AShooter_Character>(Other_Actor);
	bool bCharacter_Hit = false;
	if(Shooter_Character)
	{
		Shooter_Character->MulticastHit();
		bCharacter_Hit = true;
		Impact_Particles = Impact_Character_Particles;
	}

	Multicast_On_Hit(bCharacter_Hit);*/
}

void AProjectile::Multicast_On_Hit_Implementation(bool bCharacter_Hit)
{
	Impact_Particles = bCharacter_Hit ? Impact_Character_Particles : Impact_Obstacle_Particles;

	Destroy();
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if(Impact_Particles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Impact_Particles, GetActorTransform());
	}
	if(Impact_Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Impact_Sound, GetActorLocation());
	}
}