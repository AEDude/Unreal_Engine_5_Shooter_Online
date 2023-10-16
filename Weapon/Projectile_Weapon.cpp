// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile_Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectile_Weapon::Fire(const FVector &Hit_Target)
{
    Super::Fire(Hit_Target);
    
    if(!HasAuthority()) return;
    
    APawn* Instigator_Pawn = Cast<APawn>(GetOwner());
    const USkeletalMeshSocket* Muzzle_Flash_Socket = Get_Weapon_Mesh()->GetSocketByName(FName("MuzzleFlash"));
    if(Muzzle_Flash_Socket)
    {
        FTransform Socket_Transform = Muzzle_Flash_Socket->GetSocketTransform(Get_Weapon_Mesh());
        
        //From Muzzle Flash Socket to "Hit Location" from "Trace Under Crosshairs".
        FVector To_Target = Hit_Target - Socket_Transform.GetLocation(); 
        
        FRotator Target_Rotation = To_Target.Rotation();

        if(Projectile_Class && Instigator_Pawn)
        {
            FActorSpawnParameters Spawn_Params;
            Spawn_Params.Owner = GetOwner();
            Spawn_Params.Instigator = Instigator_Pawn;
            UWorld* World = GetWorld();
            if(World)
            {
                World->SpawnActor<AProjectile>(
                    Projectile_Class,
                    Socket_Transform.GetLocation(),
                    Target_Rotation,
                    Spawn_Params
                );
            }
        }
    }
}
