// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shooter_Online/HUD/Shooter_HUD.h"
#include "Combat_Component.generated.h"

#define TRACE_LENGTH 70000

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_ONLINE_API UCombat_Component : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombat_Component();

	friend class AShooter_Character;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	void Equip_Weapon(class AWeapon* Weapon_To_Equip);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void Set_Aiming(bool bIs_Aiming);

	UFUNCTION(Server, Reliable)
	void Server_Set_Aiming(bool bIs_Aiming);

	void Set_Sprinting(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Set_Sprinting(bool bIs_Sprinting);

	UFUNCTION()
	void OnRep_Equipped_Weapon();

	void Fire_Button_Pressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& Trace_Hit_Target);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& Trace_Hit_Target);

	void Trace_Under_Crosshairs(FHitResult& Trace_Hit_Result);

	void Set_HUD_Crosshairs(float DeltaTime);

private:
	class AShooter_Character* Character;
	class AShooter_Player_Controller* Controller;
	class AShooter_HUD* HUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_Equipped_Weapon)
	AWeapon* Equipped_Weapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(Replicated)
	bool bSprinting;

	UPROPERTY(EditAnywhere)
	float Base_Walk_Speed;
	
	UPROPERTY(EditAnywhere)
	float Aim_Walk_Speed;

	UPROPERTY(EditAnywhere)
	float Sprint_Speed;

	bool bFire_Button_Pressed;

	/**
	 * HUD & Crosshairs
	*/
	FHUDPackage HUD_Package;
	
	float Crosshair_Velocity_Factor;
	float Crosshair_In_Air_Factor; 
	float Crosshair_Aim_Factor;
	float Crosshair_Firing_Weapon_Factor;

	FVector Hit_Target;
	

	/**
	 * Aiming and FOV
	*/

	//Field of view when not aiming set to the camera's base field of view in Begin Play
	float Default_FOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float Zoomed_FOV = 33.f;

	float Current_FOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float Zoom_Interp_Speed = 25.f;

	void Interp_FOV(float DeltaTime);

public:	

		
};
