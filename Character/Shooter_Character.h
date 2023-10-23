// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Shooter_Online/Shooter_Types/Turning_In_Place.h"
#include "Shooter_Online/Interfaces/Crosshairs_Interaction_Interface.h"
#include "Shooter_Online/Shooter_Types/Combat_State.h"
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

	void Play_Reload_Montage();

	void Play_Eliminated_Montage();

	virtual void OnRep_ReplicatedMovement() override;

	void Ragdoll_On_Death();

	void Eliminated();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Eliminated();

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisable_Gameplay = false;

	UPROPERTY()
	class AShooter_Player_State* Shooter_Player_State;
	
	UFUNCTION(BlueprintImplementableEvent)
	void Show_Sniper_Scope_Widget(bool bShow_Scope);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Move_Forward(float Value);
	void Move_Right(float Value);
	void Turn(float Value);
	void Look_Up(float Value);
	void Equip_Button_Pressed();
	void Crouch_Button_Pressed();
	void Reload_Button_Pressed();
	void Aim_Button_Pressed();
	void Aim_Button_Released();
	void Aim_Offset(float DeltaTime);
	void Rotate_In_Place(float DeltaTime);
	void Calculate_AO_Pitch();
	void Simulated_Proxies_Turn();
	void Sprint_Button_Pressed();
	void Sprint_Button_Released();
	virtual void Jump() override;
	void Fire_Button_Pressed();
	void Fire_Button_Released();
	void Play_Hit_React_Montage();
	
	UFUNCTION()
	void Recieve_Damage(AActor* Damaged_Actor, float Damage, const UDamageType* Damage_Type, class AController* Instigator_Controller, AActor* Damage_Causer);
	void Update_HUD_Health();
	// Poll for any relevant classes and initialize our HUD
	void Poll_Initialize();


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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombat_Component* Combat;

	UFUNCTION(Server, Reliable)
	void Server_Equip_Button_Pressed();

	float AO_Yaw;
	float Interp_AO_Yaw;
	float AO_Pitch;
	FRotator Starting_Aim_Rotation;

	ETurning_In_Place Turning_In_Place;
	void Turn_In_Place(float DeltaTime);

	/**
	 * Animation Montages
	*/

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* Fire_Weapon_Montage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* Reload_Montage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* Hit_React_Montage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* Eliminated_Montage;



	
	void Hide_Camera_When_Character_Is_Close();
	
	UPROPERTY(EditAnywhere)
	float Camera_Threshold = 150.f;

	bool bRotate_Root_Bone;
	float Turn_Threshold = 0.5f;
	FRotator Proxy_Rotation_Last_Frame;
	FRotator Proxy_Rotation;
	float Proxy_Yaw;
	float Time_Since_Last_Movement_Replication;
	float Calculate_Speed();

	/**
	 * Player Health
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float Max_Health = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class AShooter_Player_Controller* Shooter_Player_Controller;

	bool bEliminated = false;

	FTimerHandle Eliminated_Timer;

	UPROPERTY(EditDefaultsOnly)
	float Eliminated_Delay = 3.f;

	void Eliminated_Timer_Finished();


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
	FORCEINLINE bool Should_Rotate_Root_Bone() const { return bRotate_Root_Bone; }
	FORCEINLINE bool Is_Eliminated() const { return bEliminated; }
	FORCEINLINE float Get_Health() const { return Health; }
	FORCEINLINE float Get_Max_Health() const { return Max_Health; }
	ECombat_State Get_Combat_State() const;
	FORCEINLINE UCombat_Component* Get_Combat() const { return Combat; }
	FORCEINLINE bool Get_Disable_Gameplay() const { return bDisable_Gameplay; }
};
