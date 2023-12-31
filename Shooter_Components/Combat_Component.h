// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shooter_Online/HUD/Shooter_HUD.h"
#include "Shooter_Online/Weapon/Weapon_Types.h"
#include "Shooter_Online/Shooter_Types/Combat_State.h"
#include "Combat_Component.generated.h"

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
	void Fire_Button_Pressed(bool bPressed);
	
	void Reload();

	UFUNCTION(BlueprintCallable)
	void Finish_Reloading();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	void Set_Aiming(bool bIs_Aiming);

	UFUNCTION(Server, Reliable)
	void Server_Set_Aiming(bool bIs_Aiming);

	void Start_Sprinting_Unequipped(bool bIs_Sprinting);

	void Stop_Sprinting_Unequipped(bool bIs_Sprinting);

	void Start_Sprinting_Equipped(bool bIs_Sprinting);

	void Stop_Sprinting_Equipped(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Start_Sprinting_Unequipped(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Stop_Sprinting_Unequipped(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Start_Sprinting_Equipped(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Stop_Sprinting_Equipped(bool bIs_Sprinting);

	UFUNCTION()
	void OnRep_Equipped_Weapon();

	void Sprint_Button_Pressed(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& Trace_Hit_Target);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& Trace_Hit_Target);

	void Trace_Under_Crosshairs(FHitResult& Trace_Hit_Result);

	void Set_HUD_Crosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void Server_Reload();

	void Handle_Reload();

	int32 Amount_To_Reload();

private:
	UPROPERTY()
	class AShooter_Character* Character;
	UPROPERTY()
	class AShooter_Player_Controller* Controller;
	UPROPERTY()
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

	bool bSprint_Button_Pressed;
	bool bSprint_Button_Released;
	
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

	/**
	 * Automatic Firing of Weapons
	*/
	FTimerHandle Fire_Timer;
	
	bool bCan_Fire = true;

	void Start_Fire_Timer();
	void Fire_Timer_Finished();

	bool Can_Fire();

	//Carried Ammo for the currently equipped weapon.
	UPROPERTY(ReplicatedUsing = OnRep_Carried_Ammo)
	int32 Carried_Ammo;

	UFUNCTION()
	void OnRep_Carried_Ammo();

	TMap<EWeapon_Type, int32> Carried_Ammo_Map;

	UPROPERTY(EditAnywhere)
	int32 Starting_Pistol_Ammo = 0;

	UPROPERTY(EditAnywhere)
	int32 Starting_Submachine_Gun_Ammo = 0;

	UPROPERTY(EditAnywhere)
	int32 Starting_Assult_Rifle_Ammo = 0;

	UPROPERTY(EditAnywhere)
	int32 Starting_Shotgun_Ammo = 0;

	UPROPERTY(EditAnywhere)
	int32 Starting_Sniper_Rifle_Ammo = 0;

	UPROPERTY(EditAnywhere)
	int32 Starting_Grenade_Launcher_Ammo = 0;

	UPROPERTY(EditAnywhere)
	int32 Starting_Rocket_Launcher_Ammo = 0;

	void Initialize_Carried_Ammo();

	UPROPERTY(ReplicatedUsing = OnRep_Combat_State)
	ECombat_State Combat_State = ECombat_State::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_Combat_State();

	void Update_Ammo_Values();
	

public:	

	FORCEINLINE ECombat_State Get_Combat_State() const { return Combat_State; }
};