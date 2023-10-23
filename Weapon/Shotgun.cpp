// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Weapon_Types.h"


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

