// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Weapon_Types.h"
#include "Kismet/KismetMathLibrary.h"


void AShotgun::Fire(const FVector &Hit_Target)
{
    AWeapon::Fire(Hit_Target);
    APawn* Owner_Pawn = Cast<APawn>(GetOwner());
    if(Owner_Pawn == nullptr) return;
    AController* Instigator_Controller = Owner_Pawn->GetController();
    if(!HasAuthority() && Instigator_Controller) UE_LOG(LogTemp, Warning, TEXT("INSTIGATOR_VALID"));



    const USkeletalMeshSocket* Muzzle_Flash_Socket = Get_Weapon_Mesh()->GetSocketByName("MuzzleFlash");
    if(Muzzle_Flash_Socket)
    {
        FTransform Socket_Transform = Muzzle_Flash_Socket->GetSocketTransform(Get_Weapon_Mesh());
        FVector Start = Socket_Transform.GetLocation();
        
        
        TMap<AShooter_Character*, uint32> Hit_Map;
        for(uint32 i = 0; i < Number_Of_Shot; i++)
        {
            FHitResult Fire_Hit;
            Weapon_Trace_Hit(Start, Hit_Target, Fire_Hit);

            AShooter_Character* Shooter_Character = Cast<AShooter_Character>(Fire_Hit.GetActor());
            if(Shooter_Character && HasAuthority() && Instigator_Controller)
            {
                if(Hit_Map.Contains(Shooter_Character))
                {
                    Hit_Map[Shooter_Character]++;
                }
                else
                {
                    Hit_Map.Emplace(Shooter_Character, 1);
                }
            }
            
            if(Impact_Particles)
            {
                UGameplayStatics::SpawnEmitterAtLocation(
                    GetWorld(),
                    Impact_Particles,
                    Fire_Hit.ImpactPoint,
                    Fire_Hit.ImpactNormal.Rotation()
                );
            }
            if(Hit_Sound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    this,
                    Hit_Sound,
                    Fire_Hit.ImpactPoint,
                    .7f,
                    FMath::FRandRange(-.7f, .7f)
                );
            }
        }
        for(auto Hit_Pair : Hit_Map)
        {
            if(Hit_Pair.Key && HasAuthority() && Instigator_Controller)
            {
                UGameplayStatics::ApplyDamage(
                    Hit_Pair.Key,
                    Damage * Hit_Pair.Value,
                    Instigator_Controller,
                    this,
                    UDamageType::StaticClass()
                );
            }
        }
    }
}

void AShotgun::Shotgun_Trace_End_With_Scatter(const FVector& Hit_Target, TArray<FVector>& Hit_Targets)
{
    const USkeletalMeshSocket* Muzzle_Flash_Socket = Get_Weapon_Mesh()->GetSocketByName("MuzzleFlash");
    if(Muzzle_Flash_Socket == nullptr) return;
    
    const FTransform Socket_Transform = Muzzle_Flash_Socket->GetSocketTransform(Get_Weapon_Mesh());
    const FVector Trace_Start = Socket_Transform.GetLocation();
    
    const FVector To_Target_Normalized = (Hit_Target - Trace_Start).GetSafeNormal();
    const FVector Sphere_Center = Trace_Start + To_Target_Normalized * Distance_To_Sphere;

    for(uint32 i = 0; i < Number_Of_Shot; i++)
    {
        const FVector Random_Vector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, Sphere_Radius);
        const FVector End_Location = Sphere_Center + Random_Vector;
        FVector To_End_Location = End_Location - Trace_Start;
        Trace_Start + To_End_Location * TRACE_LENGTH / To_End_Location.Size();
        
        Hit_Targets.Add(To_End_Location);
    }
}

