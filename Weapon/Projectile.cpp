// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Shooter_Online\Character\Shooter_Character.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Shooter_Online/Shooter_Online.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

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
	Collision_Box->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	Collision_Box->SetCollisionResponseToChannel(ECC_Skeletal_Mesh, ECollisionResponse::ECR_Block);
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

void AProjectile::Start_Destroy_Timer()
{
	GetWorldTimerManager().SetTimer(
        Destroy_Timer,
        this,
        &AProjectile::Destroy_Timer_Finished,
        Destroy_Time
    );
}

void AProjectile::Destroy_Timer_Finished()
{
    Destroy();
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

void AProjectile::Spawn_Trail_System()
{
	if(Trail_System)
    {
        Trail_System_Component = UNiagaraFunctionLibrary::SpawnSystemAttached(
            Trail_System,
            GetRootComponent(),
            FName(),
            GetActorLocation(),
            GetActorRotation(),
            EAttachLocation::KeepWorldPosition,
            false
        );
    }

}

void AProjectile::Explode_Damage()
{
    APawn* Firing_Pawn = GetInstigator();
    if(Firing_Pawn && HasAuthority())
    {
        AController* Firing_Controller = Firing_Pawn->GetController();
        if(Firing_Controller)
        {
            UGameplayStatics::ApplyRadialDamageWithFalloff(
                this, //World Context Object
                Damage, //Base Damage
                15.f, //Minimum Damage
                GetActorLocation(), //Origin of damage
                Damage_Inner_Radius, // Damage inner radius 
                Damage_Outer_Radius, // Damage outer radius
                1.f, // Damage falloff
                UDamageType::StaticClass(), //Damage Type Class
                TArray<AActor*>(), // Empty Array of Ignore Actors
                this, // Damage causer
                Firing_Controller //Instigator controller
            );
        }
    }
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