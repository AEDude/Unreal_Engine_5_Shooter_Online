// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Shooter_Online/Shooter_Types/Turning_In_Place.h"
#include "Shooter_Online/Interfaces/Crosshairs_Interaction_Interface.h"
#include "Shooter_Character.generated.h"

UCLASS()
class SHOOTER_ONLINE_API AShooter_Character : public ACharacter, public ICrosshairs_Interaction_Interface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooter_Character();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void Play_Fire_Montage(bool bAiming);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move_Forward(float Value);
	void Move_Right(float Value);
	void Turn(float Value);
	void Look_Up(float Value);
	void Equip_Button_Pressed();
	void Crouch_Button_Pressed();
	void Aim_Button_Pressed();
	void Aim_Button_Released();
	void Aim_Offset(float DeltaTime);
	void Sprint_Button_Pressed();
	void Sprint_Button_Released();
	virtual void Jump() override;
	void Fire_Button_Pressed();
	void Fire_Button_Released();


private:

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* Camera_Boom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* Follow_Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* Overhead_Widget;

	UPROPERTY(ReplicatedUsing = OnRep_Overlapping_Weapon)
	class AWeapon* Overlapping_Weapon;

	UFUNCTION()
	void OnRep_Overlapping_Weapon(AWeapon* Last_Weapon);

	UPROPERTY(VisibleAnywhere)
	class UCombat_Component* Combat;

	UFUNCTION(Server, Reliable)
	void Server_Equip_Button_Pressed();

	float AO_Yaw;
	float Interp_AO_Yaw;
	float AO_Pitch;
	FRotator Starting_Aim_Rotation;

	ETurning_In_Place Turning_In_Place;
	void Turn_In_Place(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* Fire_Weapon_Montage;

public:	

	void Set_Overlapping_Weapon(AWeapon* Weapon);
	bool Is_Weapon_Equipped();
	bool Is_Aiming();
	bool Is_Sprinting();
	FORCEINLINE float Get_AO_Yaw() const { return AO_Yaw;}
	FORCEINLINE float Get_AO_Pitch() const { return AO_Pitch; }
	AWeapon* Get_Equipped_Weapon();
	FORCEINLINE ETurning_In_Place Get_Turning_In_Place() const { return Turning_In_Place; }
	FVector Get_Hit_Target() const;
	FORCEINLINE UCameraComponent* Get_Follow_Camera() const { return Follow_Camera; }
};
