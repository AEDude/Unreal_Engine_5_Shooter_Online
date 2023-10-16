// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Anim_Instance.h"
#include "Shooter_Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shooter_Online/Weapon/Weapon.h"

void UShooter_Anim_Instance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    Shooter_Character = Cast<AShooter_Character>(TryGetPawnOwner());
}

void UShooter_Anim_Instance::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if(Shooter_Character == nullptr)
    {
        Shooter_Character = Cast<AShooter_Character>(TryGetPawnOwner());
    }
    if(Shooter_Character == nullptr) return;

    FVector Velocity = Shooter_Character->GetVelocity();
    Velocity.Z = 0.f;
    Speed = Velocity.Size();

    bIs_In_Air = Shooter_Character->GetCharacterMovement()->IsFalling();
    bIs_Accelerating = Shooter_Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
    bWeapon_Eqipped = Shooter_Character->Is_Weapon_Equipped();
    Equipped_Weapon = Shooter_Character->Get_Equipped_Weapon();
    bIs_Crouched = Shooter_Character->bIsCrouched;
    bAiming = Shooter_Character->Is_Aiming();
    bSprinting = Shooter_Character->Is_Sprinting();
    Turning_In_Place = Shooter_Character->Get_Turning_In_Place();

    // Offset Yaw for Strafing
    FRotator Aim_Rotation = Shooter_Character->GetBaseAimRotation();
    FRotator Movement_Rotation = UKismetMathLibrary::MakeRotFromX(Shooter_Character->GetVelocity());
    FRotator Delta_Rot = UKismetMathLibrary::NormalizedDeltaRotator(Movement_Rotation, Aim_Rotation);
    Delta_Rotation = FMath::RInterpTo(Delta_Rotation, Delta_Rot, DeltaTime, 7.f);
    Yaw_Offset = Delta_Rotation.Yaw;
    /*if(!Shooter_Character->HasAuthority() && !Shooter_Character->IsLocallyControlled())
    {
        UE_LOG(LogTemp, Warning, TEXT("Aim_Rotation Yaw %f"), Aim_Rotation.Yaw);
        UE_LOG(LogTemp, Warning, TEXT("Movement_Rotation Yaw %f"), Movement_Rotation.Yaw);
    }*/

    // Leaning
    Character_Rotation_Last_Frame = Character_Rotation;
    Character_Rotation = Shooter_Character->GetActorRotation();
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(Character_Rotation, Character_Rotation_Last_Frame);
    const float Target = -Delta.Yaw / DeltaTime;
    const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 7.f);
    Lean = FMath::Clamp(Interp, -90.f, 90.f);

    AO_Yaw = Shooter_Character->Get_AO_Yaw();
    AO_Pitch = Shooter_Character->Get_AO_Pitch();

    if(bWeapon_Eqipped && Equipped_Weapon && Equipped_Weapon->Get_Weapon_Mesh() && Shooter_Character->GetMesh() && !bSprinting)
    {
        Left_Hand_Transform = Equipped_Weapon->Get_Weapon_Mesh()->GetSocketTransform(FName("Left_Hand_Socket"), ERelativeTransformSpace::RTS_World);
        FVector Out_Position;
        FRotator Out_Rotation;
        Shooter_Character->GetMesh()->TransformToBoneSpace(FName("hand_r"), Left_Hand_Transform.GetLocation(), FRotator::ZeroRotator, Out_Position, Out_Rotation);
        Left_Hand_Transform.SetLocation(Out_Position);
        Left_Hand_Transform.SetRotation(FQuat(Out_Rotation));

        if(Shooter_Character->IsLocallyControlled())
        {
            bLocally_Controlled = true;
            //Fix Aim Offset so the muzzle of the gun is always pointed at the crosshairs when hip and ironsights aiming.
            FTransform Right_Hand_Transform = Equipped_Weapon->Get_Weapon_Mesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
            FRotator Look_At_Rotation = UKismetMathLibrary::FindLookAtRotation(Right_Hand_Transform.GetLocation(), Right_Hand_Transform.GetLocation() + (Right_Hand_Transform.GetLocation() - Shooter_Character->Get_Hit_Target()));
            Right_Hand_Rotation = FMath::RInterpTo(Right_Hand_Rotation, Look_At_Rotation, DeltaTime, 30.f);
        
            /*FTransform Muzzle_Tip_Transform = Equipped_Weapon->Get_Weapon_Mesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
            FVector Muzzle_X(FRotationMatrix(Muzzle_Tip_Transform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
            DrawDebugLine(GetWorld(), Muzzle_Tip_Transform.GetLocation(), Muzzle_Tip_Transform.GetLocation() + Muzzle_X * 1500.f, FColor::Yellow);
            DrawDebugLine(GetWorld(), Muzzle_Tip_Transform.GetLocation(), Shooter_Character->Get_Hit_Target(), FColor::Green);*/
        }
    }
}

