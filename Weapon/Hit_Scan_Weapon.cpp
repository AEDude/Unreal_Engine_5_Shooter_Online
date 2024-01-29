// Fill out your copyright notice in the Description page of Project Settings.


#include "Hit_Scan_Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"

void AHit_Scan_Weapon::Fire(const FVector &Hit_Target)
{
    Super::Fire(Hit_Target);

    APawn* Owner_Pawn = Cast<APawn>(GetOwner());
    if(Owner_Pawn == nullptr) return;
    AController* Instigator_Controller = Owner_Pawn->GetController();
    if(!HasAuthority() && Instigator_Controller) UE_LOG(LogTemp, Warning, TEXT("INSTIGATOR_VALID"));



    const USkeletalMeshSocket* Muzzle_Flash_Socket = Get_Weapon_Mesh()->GetSocketByName("MuzzleFlash");
    if(Muzzle_Flash_Socket)
    {
        FTransform Socket_Transform = Muzzle_Flash_Socket->GetSocketTransform(Get_Weapon_Mesh());
        FVector Start = Socket_Transform.GetLocation();
        
        FHitResult Fire_Hit;
        Weapon_Trace_Hit(Start, Hit_Target, Fire_Hit);
        
        AShooter_Character* Shooter_Character = Cast<AShooter_Character>(Fire_Hit.GetActor());
        if(Shooter_Character && HasAuthority() && Instigator_Controller)
        {
            UGameplayStatics::ApplyDamage(
                Shooter_Character,
                Damage,
                Instigator_Controller,
                this,
                UDamageType::StaticClass()
                );
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
                Fire_Hit.ImpactPoint
                );
        }
        if(Muzzle_Flash)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                Muzzle_Flash,
                Socket_Transform
            );
        }
        if(Fire_Sound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                Fire_Sound,
                GetActorLocation()
            );
        }
    }
}

void AHit_Scan_Weapon::Weapon_Trace_Hit(const FVector& Trace_Start, const FVector& Hit_Target, FHitResult& Out_Hit)
{
   FHitResult Fire_Hit;
   UWorld* World = GetWorld();
   if(World)
   {
        FVector End = Trace_Start + (Hit_Target - Trace_Start) * 1.25f; 

        World->LineTraceSingleByChannel(
                Out_Hit,
                Trace_Start,
                End,
                ECollisionChannel::ECC_Visibility
                );
        FVector Beam_End = End;
        if(Out_Hit.bBlockingHit)
        {
            Beam_End = Out_Hit.ImpactPoint;
        }

        DrawDebugSphere(
            GetWorld(),
            Beam_End,
            17.f,
            12,
            FColor::Green,
            true
            );

        if(Beam_Particles)
        {
            UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
                World,
                Beam_Particles,
                Trace_Start,
                FRotator::ZeroRotator,
                true
                );
            if(Beam)
            {
                Beam->SetVectorParameter(FName("Target"), Beam_End);
            }
        }
   } 
}