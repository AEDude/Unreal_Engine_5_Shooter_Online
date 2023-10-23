// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile_Bullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"


AProjectile_Bullet::AProjectile_Bullet()
{
    Projectile_Movement_Component = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	Projectile_Movement_Component->bRotationFollowsVelocity = true;
    Projectile_Movement_Component-> SetIsReplicated(true);
}

void AProjectile_Bullet::On_Hit(UPrimitiveComponent *Hit_Comp, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, FVector Normal_Impulse, const FHitResult &Hit)
{
    ACharacter* Owner_Character = Cast<ACharacter>(GetOwner());
    if(Owner_Character)
    {
        AController* Owner_Controller = Owner_Character->Controller;
        if(Owner_Character)
        {
             UGameplayStatics::ApplyDamage(Other_Actor, Damage, Owner_Controller, this, UDamageType::StaticClass());
        }
    }

    Super::On_Hit(Hit_Comp, Other_Actor, Other_Comp, Normal_Impulse, Hit);
}
