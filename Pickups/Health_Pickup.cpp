// Fill out your copyright notice in the Description page of Project Settings.


#include "Health_Pickup.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Shooter_Online/Shooter_Components/Buff_Component.h"


AHealth_Pickup::AHealth_Pickup()
{
    bReplicates = true;
}

void AHealth_Pickup::On_Sphere_Overlap(UPrimitiveComponent *Overlapped_Component, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, int32 Other_Body_Index, bool bFrom_Sweep, const FHitResult &Sweep_Result)
{
   Super::On_Sphere_Overlap(Overlapped_Component, Other_Actor, Other_Comp, Other_Body_Index, bFrom_Sweep, Sweep_Result);

    AShooter_Character* Shooter_Character = Cast<AShooter_Character>(Other_Actor);
    if(Shooter_Character)
    {
        UBuff_Component* Buff = Shooter_Character->Get_Buff();
        if(Buff)
        {
            Buff->Heal(Healing_Amount, Healing_Time);
        }
    }
    
    Destroy();
}
