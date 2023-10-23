// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile_Rocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Rocket_Movement_Component.h"


AProjectile_Rocket:: AProjectile_Rocket()
{
    Projectile_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket_Mesh"));
    Projectile_Mesh->SetupAttachment(RootComponent);
    Projectile_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    Rocket_Movement_Component = CreateDefaultSubobject<URocket_Movement_Component>(TEXT("Rocket_Movement_Component"));
    Rocket_Movement_Component->bRotationFollowsVelocity = true;
    Rocket_Movement_Component->SetIsReplicated(true);
}

void AProjectile_Rocket::BeginPlay()
{
    Super::BeginPlay();

    if(!HasAuthority())
	{
		Collision_Box->OnComponentHit.AddDynamic(this, &AProjectile_Rocket::On_Hit);
	}

    Spawn_Trail_System();
   
    if(Projectile_Loop && Looping_Sound_Attenuation)
    {
        Projectile_Loop_Component = UGameplayStatics::SpawnSoundAttached(
            Projectile_Loop,
            GetRootComponent(),
            FName(),
            GetActorLocation(),
            EAttachLocation::KeepWorldPosition,
            false,
            1.f,
            1.f,
            0.f,
            Looping_Sound_Attenuation,
            (USoundConcurrency*)nullptr,
            false
        );
    }
}

void AProjectile_Rocket::On_Hit(UPrimitiveComponent *Hit_Comp, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, FVector Normal_Impulse, const FHitResult &Hit)
{
    Explode_Damage();

    Start_Destroy_Timer();

    if(Impact_Particles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Impact_Particles, GetActorTransform());
	}
	if(Impact_Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Impact_Sound, GetActorLocation());
	}
    if(Projectile_Mesh)
    {
        Projectile_Mesh->SetVisibility(false);
    }
    if(Collision_Box)
    {
        Collision_Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if(Trail_System_Component && Trail_System_Component->GetSystemInstanceController())
    {
        Trail_System_Component->GetSystemInstanceController();
    }
    if(Projectile_Loop_Component && Projectile_Loop_Component->IsPlaying())
    {
        Projectile_Loop_Component->Stop();
    }
}

void AProjectile_Rocket::Destroyed()
{

}
