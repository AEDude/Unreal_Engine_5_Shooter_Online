// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Shooter_Online/Weapon/Weapon_Types.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
APickup::APickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Overlap_Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Overlap_Sphere"));
	Overlap_Sphere->SetupAttachment(RootComponent);
	Overlap_Sphere->SetSphereRadius(150.f);
	Overlap_Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Overlap_Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	Overlap_Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	Overlap_Sphere->AddLocalOffset(FVector(0.f, 0.f, 50.f));

	Pickup_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup_Mesh"));
	Pickup_Mesh->SetupAttachment(Overlap_Sphere);
	Pickup_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Pickup_Mesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	Pickup_Mesh->SetRenderCustomDepth(true);
	Pickup_Mesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE);

	Pickup_Effect_Component = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Pickup_Effect_Component"));
    Pickup_Effect_Component->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

	Starting_Location = GetActorLocation();

	if(HasAuthority())
	{
		GetWorldTimerManager().SetTimer(
			Bind_Overlap_Timer,
			this,
			&APickup::Bind_Overlap_Timer_Finished,
			Bind_Overlap_Time
		);
	}
}

// Called every frame
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(Pickup_Mesh)
	{
		Raise_Lower_Pickup(DeltaTime);
		Pickup_Mesh->AddLocalRotation(FRotator(0.f, 0.f, Base_Turn_Rate * DeltaTime));
	}

}

void APickup::On_Sphere_Overlap(UPrimitiveComponent* Overlapped_Component, AActor* Other_Actor, UPrimitiveComponent* Other_Comp, int32 Other_Body_Index, bool bFrom_Sweep, const FHitResult& Sweep_Result)
{

}

void APickup::Destroyed()
{
	Super::Destroyed();

	if(Pickup_Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			Pickup_Sound,
			GetActorLocation()
		);
	}

	if(Pickup_Effect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        this,
        Pickup_Effect,
        GetActorLocation(),
        GetActorRotation()
        );
    }
}

void APickup::Raise_Lower_Pickup(float DeltaTime)
{
	if(!Should_Pickup_Return())
	{
		Raise_Pickup(DeltaTime);
	}
	else
	{
		Lower_Pickup(DeltaTime);
	}

}

void APickup::Raise_Pickup(float DeltaTime)
{
	const FVector Location_To_Raise_Pickup = GetActorLocation() + (Raise_Lower_Velocity * DeltaTime);
	SetActorLocation(Location_To_Raise_Pickup);
}

void APickup::Lower_Pickup(float DeltatTime)
{
	FVector Move_Direction_Normal = Raise_Lower_Velocity.GetSafeNormal();
	Starting_Location = Starting_Location + (Move_Direction_Normal * Maximum_Move_Distance);
	SetActorLocation(Starting_Location);
	Raise_Lower_Velocity = -Raise_Lower_Velocity;
}

float APickup::Distance_Moved() const
{
    return FVector::Distance(Starting_Location, GetActorLocation());
}

bool APickup::Should_Pickup_Return() const
{
	float Pickup_Distance_Moved = Distance_Moved();
    return Pickup_Distance_Moved > Maximum_Move_Distance;
}

void APickup::Bind_Overlap_Timer_Finished()
{
		Overlap_Sphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::On_Sphere_Overlap);
}
