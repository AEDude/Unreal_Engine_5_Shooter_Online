// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup_Spawn_Point.h"
#include "Pickup.h"

// Sets default values
APickup_Spawn_Point::APickup_Spawn_Point()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void APickup_Spawn_Point::BeginPlay()
{
	Super::BeginPlay();
	Start_Spawn_Pikup_Timer((AActor*)nullptr);
	
}
// Called every frame
void APickup_Spawn_Point::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup_Spawn_Point::Spawn_Pickup()
{
	int32 Number_Of_Pickup_Classes = Pickup_Classes.Num();
	if(Number_Of_Pickup_Classes > 0)
	{
		int32 Selection = FMath::RandRange(0, Number_Of_Pickup_Classes - 1);
		Spawned_Pickup =  GetWorld()->SpawnActor<APickup>(Pickup_Classes[Selection], GetActorTransform());

		if(HasAuthority() && Spawned_Pickup)
		{
			Spawned_Pickup->OnDestroyed.AddDynamic(this, &APickup_Spawn_Point::Start_Spawn_Pikup_Timer);
		}
	}
}

void APickup_Spawn_Point::Start_Spawn_Pikup_Timer(AActor* Destroyed_Actor)
{
	const float Spawn_Time = FMath::FRandRange(Spawn_Pickup_Timer_Minimum, Spawn_Pickup_Timer_Maximum);
	GetWorldTimerManager().SetTimer(
		Spawn_Pickup_Timer,
		this,
		&APickup_Spawn_Point::Spawn_Pickup_Timer_Finished,
		Spawn_Time
	);

}

void APickup_Spawn_Point::Spawn_Pickup_Timer_Finished()
{
	if(HasAuthority())
	{
		Spawn_Pickup();
	}
}
