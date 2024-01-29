// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Shooter_Online/Shooter_Types/Turning_In_Place.h"
#include "Shooter_Anim_Instance.generated.h"

class AShooter_Character;
class AWeapon;

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API UShooter_Anim_Instance : public UAnimInstance
{
	GENERATED_BODY()

public:
	//Override the "Begin Play" of the animation instance class.
	virtual void NativeInitializeAnimation() override;

	//Override the "Tick" of animation instance class.
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	/*"meta = (AllowPrivateAccess = "true")" specifier must be initialized on all private variables that will be called in blueprint*/

	//Stores the character using this animation instance.
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	AShooter_Character* Shooter_Character;
	
	//Stores the speed aka velocity of the character.
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed{};

	//Stores whether or not the character is in the air.
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIs_In_Air{};
	
	//Stores whether or not there is input for moving the character.
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIs_Accelerating{};

	//Checks to see if a weapon is equipped by accessing the "Equipped_Weapon" "AWeapon" pointer 
	// in the combat component. If said pointer == null then there is no weapon equipped,
	//If the pointer returns valid there is a weapon equipped so the animations being used
	// need to be changed accordingly in the editor via ABP.
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeapon_Eqipped{};

	//Variable to recieve information of the "Equipped_Weapon" from "AShooter_Character".
	AWeapon* Equipped_Weapon;

	//Sets wheather "AShooter_Character" is crouching or not. Gets the value from within "&AShooter_Character::Crouch_Button_Pressed()".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIs_Crouched{};

	//Set by the return value of &AShooter_Character::Is_Aiming, which is set by "&UCombat_Component::Set_Aiming" which is set by 
	//"&AShooter_Character::Aim_Button_Pressed" && "&AShooter_Character::Aim_Button_Released".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming{};

	//This variable is set by the return value of &AShooter_Character::Is_Sprinting, which is set by 
	//"&UCombat_Component::Set_Sprinting" which is set by "&AShooter_Character::Sprint_Button_Pressed" && 
	//"&AShooter_Character::Sprint_Button_Released".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bSprinting{};
	
	//Variable used to drive the strafing of the character within "ABP_Shooter".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Yaw_Offset{};
	
	//Variable used to drive the "Lean" angle of the character within "ABP_Shooter".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean{};

	//Used to calculate the lean for the character (See "Equipped_Run" Blendspace to visualize "Lean").
	FRotator Character_Rotation_Last_Frame{};
	FRotator Character_Rotation{};
	
	//Store the difference in rotations of the character for use in "Yaw_Offset"
	FRotator Delta_Rotation{};

	//Variable which will be used to drive the Aim_Offset Yaw in the "Hip_Aim_Offset" and "Iron_Sights_Aim_Offset".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw{};

	//Variable which will be used to drive the Aim_Offset Pitch in the "Hip_Aim_Offset" and "Iron_Sights_Aim_Offset".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch{};

	//Use FABRIK to place the left hand on the appropriate location on the weapon.
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform Left_Hand_Transform{};

	//Variable which is used to determine whether the character should "Turn_In_Place"
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurning_In_Place Turning_In_Place{};

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator Right_Hand_Rotation{};
	//Used to determine wheather the character is locally controlled or a simulated proxy.
	//This variable will be used along side "bRotate_Root_Bone" to determine how the simulated proxies "Turn_In_Place".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocally_Controlled{};
	//Used to determine wheather the character should rotate it's root bone when turning in place or not.
	//This depends on wheather the character is a simulated proxy or "IsLocallyControlled()".
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotate_Root_Bone{};
	//Uded to determine wheathe the character is dead.
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bEliminated{};

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUse_FABRIK{};
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUse_Aim_Offsets{};

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bTransform_Right_Hand{};
};
