// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Anim_Instance.h"
#include "Shooter_Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shooter_Online/Weapon/Weapon.h"
#include "Shooter_Online/Shooter_Types/Combat_State.h"

void UShooter_Anim_Instance::NativeInitializeAnimation()
{   //"Super" calls the original version of the function before it is overriden.
    Super::NativeInitializeAnimation();
    
    //Cast "TryGetPawnOwner()" to AShooter_Character data type and store it in the character pointer. 
    Shooter_Character = Cast<AShooter_Character>(TryGetPawnOwner());
}

void UShooter_Anim_Instance::NativeUpdateAnimation(float DeltaTime)
{
    //"Super" calls the original version of the function before it is overriden.
    Super::NativeUpdateAnimation(DeltaTime);

    //Making sure "Shooter_Character" doesn't return a null pointer because "TryGetPawnOwner()" has to succeed in order to use the Animation Instance. 
    if(Shooter_Character == nullptr)
    {
        Shooter_Character = Cast<AShooter_Character>(TryGetPawnOwner());
    }
    if(Shooter_Character == nullptr) return; //"Shooter_Character" needs to be a valid pointer to the pawn. Therefore if it is a null pointer we return.

    //Setting the speed of the character.
    FVector Velocity{Shooter_Character->GetVelocity()};
    //Don't need to include the velocity of the Z axis into the speed. Only the lateral speed is needed.
    Velocity.Z = 0.f;
    //Sets speed.
    Speed = Velocity.Size();

    //Using the inherited function "IsFalling()" by first accessing the Character Movement Component.
    //This will notify the program if the character is in the air.
    bIs_In_Air = Shooter_Character->GetCharacterMovement()->IsFalling();

    //Accessing "GetCurrentAcceleration()" from the Character Movement Component to get the size of the acceleration. 
    //If the value is greater than 0.f, there is input from the controller to move the character and thus the character (bIs_Accelerating).
    bIs_Accelerating = Shooter_Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
    
    //Accesses "Is_Weapon_Equipped()" which is within the "AShooter_Character" (return a bool) to determine wheather a weapon is equipped.
    bWeapon_Eqipped = Shooter_Character->Is_Weapon_Equipped();
    
    //Accesses the "Equipped_Weapon" from the return of the getter fucntion found within "&AShooter_Character::Get_Equipped_Weapon".
    Equipped_Weapon = Shooter_Character->Get_Equipped_Weapon();
    
    //Sets wheather "AShooter_Character" is crouching or not. Gets the value from within "&AShooter_Character::Crouch_Button_Pressed".
    //Enable "Can Crouch" from the "AShooter_Character" Character Blueprint.
    bIs_Crouched = Shooter_Character->bIsCrouched;
    //Sets the pose of the character to aim down sights or aim at the hip.
    bAiming = Shooter_Character->Is_Aiming();
    //Sets the animation to play when the character is sprinting
    bSprinting = Shooter_Character->Is_Sprinting();
    //Uses the value retrieved from getter function found in "&AShooter_Character::Get_Turning_In_Place" to determine whether 
    //to play the "Turning_In_Place" animations or not.
    Turning_In_Place = Shooter_Character->Get_Turning_In_Place();
    
    //Uses the value retrieved from the getter function "Should_Rotate_Root_Bone()" which may be found within "&AShooter_Character::Should_Rotate_Root_Bone"
    //to determine if the character should rotate it's root bone while "Turning_in_Place."
    bRotate_Root_Bone = Shooter_Character->Should_Rotate_Root_Bone();
    
    //Uses the value retrieved from the getter function "Is_Eliminated()" which may be found within "&AShooter_Character::Is_Eliminated"
    //to determine if the character is dead"
    bEliminated = Shooter_Character->Is_Eliminated();

    
    /*Yaw_Offset for Strafing*/
    //Character uses controller yaw for determining which direction is forward.
    
    //Base rotatation of the camera (mouse input) around the character. This is a global rotation so the 
    //value is "0" when the character is facing (forwards) towards the Worlds X axis positive direction.
    //Increases when the camera rotates to the right and decreases into to the negative values(down to -180)
    //when the camera rotates to the left. It snaps to 0 when it passes -180 (where it first started).
    FRotator Aim_Rotation{Shooter_Character->GetBaseAimRotation()};
    
    //Using "MakeRotFromX" which needs a direction vector (using the direction of the veloxity) to
    //determine the Yaw rotation of the character's movement. This is a global rotation so the 
    //value is "0" when the character is facing (forwards) towards the Worlds X axis positive direction.
    //Increases when the character rotates to the right and decreases into to the negative values (down to -180)
    //when the character rotates to the left. It snaps to 0 when it passes -180 (where it first started).
    FRotator Movement_Rotation{UKismetMathLibrary::MakeRotFromX(Shooter_Character->GetVelocity())};
    
    //Use "NormalizedDeltaRotator" to calculate the difference between "Aim_Rotation" && "Movement_Rotation".
    FRotator Delta_Rot{UKismetMathLibrary::NormalizedDeltaRotator(Movement_Rotation, Aim_Rotation)};
    
    //Smoothen out the rotation of the strafing from -180 to 180 taking the shortest route to dampen snapping
    Delta_Rotation = FMath::RInterpTo(Delta_Rotation, Delta_Rot, DeltaTime, 7.f);
   
    //This will be used in the "ABP_Shooter" to drive the strafing of the character.
    Yaw_Offset = Delta_Rotation.Yaw;
    
    /*if(!Shooter_Character->HasAuthority() && !Shooter_Character->IsLocallyControlled())
    {
        UE_LOG(LogTemp, Warning, TEXT("Aim_Rotation Yaw %f"), Aim_Rotation.Yaw);
        UE_LOG(LogTemp, Warning, TEXT("Movement_Rotation Yaw %f"), Movement_Rotation.Yaw);
    }*/


    /*Leaning for when the character (Is_Acceleraring) && (!bIs_Aiming)*/
    
    
    //Has to do with the "Yaw_Offset" of the character and the character's rotation from the previous frame.

    //The value of "Character_Rotation_Last_Frame" will always be a little begind the value of "Character_Rotation".
    //This is because once "Character_Rotation" updates on the next tick, "Character_Rotation_Last_Frame" will not update until
    //the tick after that. Hence causing it to always be the previous value of "Character_Rotation" current value.
    Character_Rotation_Last_Frame = Character_Rotation;
    
    //The rotation of the "RootComponent"/Capsule Component
    Character_Rotation = Shooter_Character->GetActorRotation();
    
    //Use "NormalizedDeltaRotator" to calculate the difference between "Character_Rotation" && "Character_Rotation_Last_Frame".
    const FRotator Delta{UKismetMathLibrary::NormalizedDeltaRotator(Character_Rotation, Character_Rotation_Last_Frame)};
    
    //Bacause "Delta" is a very small value, data type "double" is used to store it.
    const double Target{-Delta.Yaw / DeltaTime};
    
    //Smoothen out the angle transition of the Target.
    const double Interp{FMath::FInterpTo(Lean, Target, DeltaTime, 7.f)};

    //Clamp the max lean angle of the character
    Lean = FMath::Clamp(Interp, -90.f, 90.f);


    //Set AO_Yaw && AO_Pitch to the getters located within "AShooter_Character." 
    AO_Yaw = Shooter_Character->Get_AO_Yaw();
    AO_Pitch = Shooter_Character->Get_AO_Pitch();


   
    /*For transforming the left && right Hand to the appropriate locations on the weapons*/

    //Safety check
    if(bWeapon_Eqipped && Equipped_Weapon && Equipped_Weapon->Get_Weapon_Mesh() && Shooter_Character->GetMesh())
    {
        //Storing the "Left_Hand_Socket" from the weapon mesh into the FTransform variable "Left_Hand_Transform using the World Space transform 
        //of the socket on the weapon. Needs to converted to the Bone Space of the character's skeletal mesh.
        Left_Hand_Transform = Equipped_Weapon->Get_Weapon_Mesh()->GetSocketTransform(FName("Left_Hand_Socket"), ERelativeTransformSpace::RTS_World);
        
        //Used to recieve the conversion transform and rotation data performed by "TransformToBoneSpace()"
        FVector Out_Position{};
        FRotator Out_Rotation{};
        
        

        /*When using FABRIK "BoneSpace" will be used as the "Effector Transform Space"*/


        /*Need to get the "End Effector Bone" on the character (hand_r) so it can be used to transform the "Left_Hand_Socket" to it Bone Space. This will enable FABRIK to be used to
        attach the "Tip Bone" (left hand) onto the "Left_Hand_Socket" while keeping it's original bone space. The goal is to have the "Left_Hand_Socket" and in return the "Tip Bone" (left hand) 
        which is attached to the weapon by FABRIK, always follow the "End Effector Bone" (the right hand) which is on the character. (Basically the "Tip Bone" left hand needs to be parented to the 
        Left_Hand_Socket which is on the weapon and the "Left_Hand_Socket" which is on the weapon needs to be parented to the "End Effector Bone" (the right hand) which is on the character.) 
        FABRIK makes this all possible*/

        //"TransformToBoneSpace()" takes the input parameters: 1. of the bone which will be used to get the bonespace to covert to, 2.the input which will be converted to the bone space of (1.),
        //3. an FRotator which will not be used so "FRotator::ZeroRotator" will fill in the space, 4. FVector& which is not const so it will be used to pass out the inforamtion of the conversion transform,
        //and 5. FRotator& which is not const so it too will be used to pass out the conversion rotation.
        Shooter_Character->GetMesh()->TransformToBoneSpace(FName("hand_r"), Left_Hand_Transform.GetLocation(), FRotator::ZeroRotator, Out_Position, Out_Rotation);
        
        //Apply the conversion values which "TransformToBoneSpace()" generated to the "Left_Hand_Transform" FABRIK will be used to attach the left hand to this "Left_Hand_Transform". 
        Left_Hand_Transform.SetLocation(Out_Position);
        //"SetRotation" requires an "FQuat" however this can be done by including "FQuat" in the input parameter.
        Left_Hand_Transform.SetRotation(FQuat(Out_Rotation));

        if(Shooter_Character->IsLocallyControlled())
        {
            bLocally_Controlled = true;
            //Fix Aim Offset so the muzzle of the gun is always pointed at the crosshairs when hip and ironsights aiming.
            FTransform Right_Hand_Transform{Equipped_Weapon->Get_Weapon_Mesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World)};
            FRotator Look_At_Rotation{UKismetMathLibrary::FindLookAtRotation(Right_Hand_Transform.GetLocation(), Right_Hand_Transform.GetLocation() + (Right_Hand_Transform.GetLocation() - Shooter_Character->Get_Hit_Target()))};
            Right_Hand_Rotation = FMath::RInterpTo(Right_Hand_Rotation, Look_At_Rotation, DeltaTime, 30.f);
        
            /*FTransform Muzzle_Tip_Transform = Equipped_Weapon->Get_Weapon_Mesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
            FVector Muzzle_X(FRotationMatrix(Muzzle_Tip_Transform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
            DrawDebugLine(GetWorld(), Muzzle_Tip_Transform.GetLocation(), Muzzle_Tip_Transform.GetLocation() + Muzzle_X * 1500.f, FColor::Yellow);
            DrawDebugLine(GetWorld(), Muzzle_Tip_Transform.GetLocation(), Shooter_Character->Get_Hit_Target(), FColor::Green);*/
        }
    }

    bUse_FABRIK = Shooter_Character->Get_Combat_State() == ECombat_State::ECS_Unoccupied;
    bUse_Aim_Offsets = Shooter_Character->Get_Combat_State() == ECombat_State::ECS_Unoccupied;
    bTransform_Right_Hand = Shooter_Character->Get_Combat_State() == ECombat_State::ECS_Unoccupied;
}

