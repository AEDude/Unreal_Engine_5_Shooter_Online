// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shooter_Online/HUD/Shooter_HUD.h"
#include "Shooter_Online/Weapon/Weapon_Types.h"
#include "Shooter_Online/Shooter_Types/Combat_State.h"
#include "Combat_Component.generated.h"

class AWeapon;
class AShooter_Character;
class AShooter_Player_Controller;
class AShooter_HUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SHOOTER_ONLINE_API UCombat_Component : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombat_Component();

	/*"AShooter_Character" and "UCombat_Component" are dependent on each other, therefore "AShooter_Character" gets full access 
	//to all of the vairables and functions decalred on "UCombat_Component".*/
	friend class AShooter_Character;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	void Spawn_Default_Weapon();

	//Equips the weapon. "Overlapping_Weapon" in "AShooter_Character" passes in information to "Weapon_To_Equip".
	void Equip_Weapon(AWeapon* Weapon_To_Equip);
	void Swap_Weapons();
	void Fire_Button_Pressed(bool bPressed);
	
	void Reload();

	UFUNCTION(BlueprintCallable)
	void Finish_Reloading();

	UFUNCTION(BlueprintCallable)
	void Shotgun_Shot_Reload();

	void Jump_To_Shotgun_End();

	UFUNCTION(BlueprintCallable)
	void Throw_Grenade_Finished();

	UFUNCTION(BlueprintCallable)
	void Launch_Grenade();

	UFUNCTION(Server, Reliable)
	void Server_Launch_Grenade(const FVector_NetQuantize& Target);

	void Pickup_Ammo(EWeapon_Type Weapon_Type, int32 Ammo_Amount);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	//Sets wheather the character is aiming. Handles everyhting client and server side.
	void Set_Aiming(bool bIs_Aiming);

	//A remote procedure call that will ask the server for permission to aim a weapon for the "AShooter_Character" which called 
	//*Inset_Function Here*.
	//RPC must be declared with a "UFUNCTION" macro paired with the "UFUNCTION" specifier "Server".
	//When an RPC is declared it must be initialized as "Reliable" or "Unreliable".
	//"Reliable" RPCs will always be executed when called. On the other hand, depending on network conditions, "Unreliable" RPCs may fail execution.
	//As the packets of data may be dropped during transit over the network.
	//Reliable RPCs get confirmation when the server recieves the request. If there is no confirmation, the "Reliable" RPC will be sent again until
	//a confirmation from the server is recieved.
	//"Reliable RPCs should be used sparingly (only on high priority functions)"
	//RPCs may be sent from client to server or from server to client.
	UFUNCTION(Server, Reliable)
	void Server_Set_Aiming(bool bIs_Aiming);

	UFUNCTION(Server, Reliable)
	void Server_Start_Sprinting_Unequipped(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Stop_Sprinting_Unequipped(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Start_Sprinting_Equipped(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Stop_Sprinting_Equipped(bool bIs_Sprinting);

	UFUNCTION(Server, Reliable)
	void Server_Fire(const FVector_NetQuantize& Trace_Hit_Target);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Fire(const FVector_NetQuantize& Trace_Hit_Target);

	UFUNCTION(Server, Reliable)
	void Server_Reload();

	UFUNCTION(Server, Reliable)
	void Server_Throw_Grenade();
	

	void Start_Sprinting_Unequipped(bool bIs_Sprinting);

	void Stop_Sprinting_Unequipped(bool bIs_Sprinting);

	void Start_Sprinting_Equipped(bool bIs_Sprinting);

	void Stop_Sprinting_Equipped(bool bIs_Sprinting);
	
	UFUNCTION()
	void OnRep_Equipped_Weapon();

	UFUNCTION()
	void On_Rep_Secondary_Weapon();

	void Set_Sprinting(bool bPressed);

	void Fire();

	void Fire_Hit_Scan_Weapon();

	void Fire_Projectile_Weapon();

	void Fire_Shotgun();

	void Local_Fire(const FVector_NetQuantize& Trace_Hit_Target);

	void Trace_Under_Crosshairs(FHitResult& Trace_Hit_Result);

	void Set_HUD_Crosshairs(float DeltaTime);

	void Handle_Reload();

	int32 Amount_To_Reload();

	void Throw_Grenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> Grenade_Class;

	void Drop_Equipped_Weapon();

	void Attach_Actor_To_Right_Hand(AActor* Actor_To_Attach);

	void Attach_Actor_To_Left_Hand(AActor* Actor_To_Attach);

	void Attach_Actor_To_Back(AActor* Actor_To_Attach);

	void Update_Carried_Ammo();
	
	//"Overlapping_Weapon" in "AShooter_Character" passes in information to "Weapon_To_Equip".
	void Play_Equip_Weapon_Sound(AWeapon* Weapon_To_Equip);

	void Reload_Empty_Magazine();

	void Show_Attached_Grenade(bool bShow_Grenade);

	//"Overlapping_Weapon" in "AShooter_Character" passes in information to "Weapon_To_Equip".
	void Equip_Primary_Weapon(AWeapon* Weapon_To_Equip);
	//"Overlapping_Weapon" in "AShooter_Character" passes in information to "Weapon_To_Equip".
	void Equip_Secondary_Weapon(AWeapon* Weapon_To_Equip);

private:

	//The Combat Component needs access to "AShooter_Character", so instead of casting a pointer will do.
	UPROPERTY()
	AShooter_Character* Character;
	UPROPERTY()
	AShooter_Player_Controller* Controller;
	UPROPERTY()
	AShooter_HUD* HUD;
	
	//Variable for storing the weapon currently equipped by "AShooter_Character".
	//Is replicated using "OnRep_Equipped_Weapon" so all clients can be in the correct poses on all computers.
	//Shooter_Anim_Instance relies on this replication.
	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(ReplicatedUsing = OnRep_Equipped_Weapon)
	AWeapon* Equipped_Weapon;

	//Variable for storing the weapon currently equipped by "AShooter_Character".
	//Is replicated using "OnRep_Equipped_Weapon" so all clients can be in the correct poses on all computers.
	//Shooter_Anim_Instance relies on this replication.
	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(ReplicatedUsing = On_Rep_Secondary_Weapon)
	AWeapon* Secondary_Weapon;

	//Sets the value for "&AShooter_Character::Is_Aiming()" to true or false. 
	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(Replicated)
	bool bAiming{};

	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(Replicated)
	bool bSprinting{};

	UPROPERTY(EditAnywhere)
	float Base_Walk_Speed{};
	
	UPROPERTY(EditAnywhere)
	float Aim_Walk_Speed{};

	UPROPERTY(EditAnywhere)
	float Sprint_Speed{};

	bool bSprint_Button_Pressed{};
	bool bSprint_Button_Released{};
	
	bool bFire_Button_Pressed{};

	/**
	 * HUD & Crosshairs
	*/
	FHUDPackage HUD_Package;
	
	float Crosshair_Velocity_Factor{};
	float Crosshair_In_Air_Factor{}; 
	float Crosshair_Aim_Factor{};
	float Crosshair_Firing_Weapon_Factor{};

	FVector Hit_Target{};

	/**
	 * Default Weapon
	*/

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AWeapon> Default_Weapon_Class;
	

	/**
	 * Aiming and FOV
	*/

	//Field of view when not aiming set to the camera's base field of view in Begin Play
	float Default_FOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float Zoomed_FOV{33.f};

	float Current_FOV{};

	UPROPERTY(EditAnywhere, Category = "Combat")
	float Zoom_Interp_Speed{25.f};

	void Interp_FOV(float DeltaTime);

	/**
	 * Automatic Firing of Weapons
	*/
	FTimerHandle Fire_Timer{};
	
	bool bCan_Fire{true};

	void Start_Fire_Timer();
	void Fire_Timer_Finished();

	bool Can_Fire();

	//Carried Ammo for the currently equipped weapon.
	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(ReplicatedUsing = OnRep_Carried_Ammo)
	int32 Carried_Ammo;

	UFUNCTION()
	void OnRep_Carried_Ammo();

	TMap<EWeapon_Type, int32> Carried_Ammo_Map;

	UPROPERTY(EditDefaultsOnly)
	int32 Max_Carried_Ammo{700};

	UPROPERTY(EditDefaultsOnly)
	int32 Starting_Pistol_Ammo{12};

	UPROPERTY(EditDefaultsOnly)
	int32 Starting_Submachine_Gun_Ammo{29};

	UPROPERTY(EditDefaultsOnly)
	int32 Starting_Assult_Rifle_Ammo{32};

	UPROPERTY(EditDefaultsOnly)
	int32 Starting_Shotgun_Ammo{8};

	UPROPERTY(EditDefaultsOnly)
	int32 Starting_Sniper_Rifle_Ammo{12};

	UPROPERTY(EditDefaultsOnly)
	int32 Starting_Grenade_Launcher_Ammo{4};

	UPROPERTY(EditDefaultsOnly)
	int32 Starting_Rocket_Launcher_Ammo{3};

	void Initialize_Carried_Ammo();

	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(ReplicatedUsing = OnRep_Combat_State)
	ECombat_State Combat_State = ECombat_State::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_Combat_State();

	void Update_Ammo_Values();

	void Update_Shotgun_Ammo_Values();

	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(ReplicatedUsing = On_Rep_Grenades)
	int32 Grenades{3};

	UPROPERTY(EditDefaultsOnly)
	int32 Maximum_Grenades{7};

	UFUNCTION()
	void On_Rep_Grenades();

	void Update_HUD_Grenades();
	

public:	

	FORCEINLINE ECombat_State Get_Combat_State() const { return Combat_State; }
	FORCEINLINE int32 Get_Grenades() const { return Grenades; }
	bool Should_Swap_Weapons();
};