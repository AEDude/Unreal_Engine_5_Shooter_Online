// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile_Grenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"


AProjectile_Grenade::AProjectile_Grenade()
{
    Projectile_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
    Projectile_Mesh->SetupAttachment(RootComponent);
    Projectile_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Projectile_Movement_Component = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	Projectile_Movement_Component->bRotationFollowsVelocity = true;
    Projectile_Movement_Component-> SetIsReplicated(true);
    Projectile_Movement_Component->bShouldBounce = true;
}

void AProjectile_Grenade::BeginPlay()
{
    AActor::BeginPlay();

     Start_Destroy_Timer();
     Spawn_Trail_System();

    Projectile_Movement_Component->OnProjectileBounce.AddDynamic(this, &AProjectile_Grenade::On_Bounce);
}

void AProjectile_Grenade::On_Bounce(const FHitResult &ImpactResult, const FVector &ImpactVelocity)
{
    if(Bounce_Sound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            Bounce_Sound,
            GetActorLocation()
        );
    }
}

void AProjectile_Grenade::Destroyed()
{
    Explode_Damage();

    Super::Destroyed();
}
