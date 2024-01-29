// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Shooter_Online/Shooter_Types/Turning_In_Place.h"
#include "Shooter_Online/Interfaces/Crosshairs_Interaction_Interface.h"
#include "Shooter_Online/Shooter_Types/Combat_State.h"
#include "Shooter_Character.generated.h"

struct FInputActionValue;
class AShooter_Player_Controller;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class AWeapon;
class UCombat_Component;
class UBuff_Component;
class UAnimMontage;
class AShooter_Player_State;


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

	/*This function must be overriden on any class which replicates variables.*/
	//Register the variables which are to be replicated within the definition of this function.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	//Initialize the null pointers located within the component classes that point to "AShooter_Character" 
	//to point to this class as early as possible. This helps with mitigating casting.
	virtual void PostInitializeComponents() override;

	void Play_Fire_Montage(bool bAiming);

	void Play_Reload_Montage();

	void Play_Eliminated_Montage();

	void Play_Throw_Grenade_Montage();

	//Inherited from the AActor class "ReplicatedMovement" which using the rep notify "OnRep_ReplicatedMovement".
	virtual void OnRep_ReplicatedMovement() override;

	void Ragdoll_On_Death();

	void Eliminated();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Eliminated();

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisable_Gameplay = false;
	
	//Implemented in the event graph of the Character Blueprint.
	//Uses the value from the input parameter in "&UCombat_Component::Set_Aiming()"
	//which is passed in by "&AShooter_Character::Aim_Button_Pressed()" to determine
	//Whether or not the aiming is occuring with a sniper rifle.
	UFUNCTION(BlueprintImplementableEvent)
	void Show_Sniper_Scope_Widget(bool bShow_Scope);

	void Update_HUD_Health();

	void Update_HUD_Armor();

	void Update_HUD_Ammo();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	/*ENHANCED INPUT UPDATE*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* Shooter_Online_Mapping_Context;

	void Add_Input_Mapping_Context(UInputMappingContext* Context_To_Add, int32 In_Priority);

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Move_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Look_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Equip_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Crouch_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Reload_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Aim_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Sprint_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Jump_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Fire_Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* Grenade_Action;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Equip_Button_Pressed();
	void Crouch_Button_Pressed();
	void Reload_Button_Pressed();
	void Aim_Button_Pressed();
	void Aim_Button_Released();
	void Aim_Offset(float DeltaTime);
	void Rotate_In_Place(float DeltaTime);
	void Calculate_AO_Pitch();
	//Handles the turn in place for the simulated proxies.
	void Simulated_Proxies_Turn();
	void Sprint_Button_Pressed();
	void Sprint_Button_Released();
	virtual void Jump() override;
	void Fire_Button_Pressed();
	void Fire_Button_Released();
	void Play_Hit_React_Montage();
	void Grenade_Button_Pressed();
	void Drop_Or_Destroy_Weapon(AWeapon* Weapon);
	void Drop_Or_Destroy_Weapons();
	
	UFUNCTION()
	void Recieve_Damage(AActor* Damaged_Actor, float Damage, const UDamageType* Damage_Type, class AController* Instigator_Controller, AActor* Damage_Causer);

	// Poll for any relevant classes and initialize our HUD
	void Poll_Initialize();


private:

	//Spring arm for Follow_Camera.
	UPROPERTY(VisibleAnywhere, Category = Camera)
	USpringArmComponent* Camera_Boom;

	//Camera which will be attached to Camera_Boom.
	UPROPERTY(VisibleAnywhere, Category = Camera)
	UCameraComponent* Follow_Camera;

	/*"meta = (AllowPrivateAccess = "true")" specifier must be initialized on all private variables that will be called in blueprint*/
	//The players ID will be displayed using this.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Overhead_Widget;

	//Stores the weapon which "Ashooter_Character" overlaps with.
	/*This pointer variable needs to be replicated so that when any change happens to the "AWeapon" class on the server, 
	(the server has authority over said class) said change will "replicate" to all clients.*/
	//Must use the "ReplicatedUsing =" specifier when replicating a variable using a Rep Notify.
	UPROPERTY(ReplicatedUsing = OnRep_Overlapping_Weapon)
	AWeapon* Overlapping_Weapon;

	//Rep notify which will be called on the specific client that "Overlapping_Weapon" replicates to.
	//Rep notifies don't get called on the server. They only get called when the variable replicates. 
	//Replication only works one way from server to client. Therefore this rep notify will never be called on the server.
	//The only input parameter which may be passed into a rep notify is that of the same data type which is being replicated.
	//Said input parameter stores the last value of the variable which is being replicated before the most recent replication occurs.
	//Must be declared with a "UFUNCTION()" macro. 
	UFUNCTION()
	void OnRep_Overlapping_Weapon(AWeapon* Last_Weapon);

	//Component which will do all the heavy lifting for anything combat related.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UCombat_Component* Combat;

	UPROPERTY(VisibleAnywhere)
	UBuff_Component* Buff;

	//A remote procedure call that will ask the server for permission to equip a weapon to the "AShooter_Character" which called 
	//"Equip_Button_Pressed".
	//RPC must be declared with a "UFUNCTION" macro paired with the "UFUNCTION" specifier "Server".
	//When an RPC is declared it must be initialized as "Reliable" or "Unreliable".
	//"Reliable" RPCs will always be executed when called. On the other hand, depending on network conditions, "Unreliable" RPCs may fail execution.
	//As the packets of data may be dropped during transit over the network.
	//Reliable RPCs get confirmation when the server recieves the request. If there is no confirmation, the "Reliable" RPC will be sent again until
	//a confirmation from the server is recieved.
	//"Reliable RPCs should be used sparingly (only on high priority functions)"
	//RPCs may be sent from client to server or from server to client.
	UFUNCTION(Server, Reliable)
	void Server_Equip_Button_Pressed();

	//Variable which will be used to drive the "AO_Yaw" in "UShooter_Anim_Instance".
	float AO_Yaw{};
	
	//Initially set to the same value as "AO_Yaw". However when the controllers rotation.Yaw is increasing or decreasing, "Interp_AO_Yaw"
	//will be interpolated to "0". After this there can be a check to see if "Turning_In_Place" left or right has been activated via enum class 
	//"ETurning_In_Place" and if it has then "AO_Yaw can be set to the value of "Interp_AO_Yaw" which is now "0", so that the lower body can "Turn_In_Place" while
	//the upper body remains still.
	float Interp_AO_Yaw{};
	
	//Variable which will be used to drive the "AO_Pitch" in "UShooter_Anim_Instance".
	float AO_Pitch{};
	
	//Variable which stores Controller rotation Yaw when the camera is absolutely behind the character and the character is facing absolutely forward.
	FRotator Starting_Aim_Rotation{};

	//Variable "pointing to the enum class which holds all the states for "Turning_In_Place"
	ETurning_In_Place Turning_In_Place;
	
	//Function which handles the logic for the enum class "ETurning_In_Place".
	void Turn_In_Place(float DeltaTime);

	/**
	 * Animation Montages
	*/

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* Fire_Weapon_Montage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* Reload_Montage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* Hit_React_Montage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* Eliminated_Montage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* Throw_Grenade_Montage;

	
	void Hide_Camera_When_Character_Is_Close();
	
	UPROPERTY(EditAnywhere)
	float Camera_Threshold = 150.f;

	//For use with simulated proxies. Simulated proxies will not depend on this node within the "ABP_Shooter".
	bool bRotate_Root_Bone;
	//Maximum value for "AO_Yaw" to reach on a simulated proxy before "Tunring_In_Place" is activated.
	float Turn_Threshold = 0.5f;
	//Amount that the simulated proxy has rotated since the last tick.
	FRotator Proxy_Rotation_Last_Frame{};
	//For the current tick. Will be used to calculate the deta between itself and "Proxy_Rotation_Last_Frame".
	FRotator Proxy_Rotation{};
	//Will be used to store the delta of between "Proxy_Rotation" and "Proxy_Rotation_Last_Frame".
	float Proxy_Yaw{};
	//Keeps track of the time passed since the simulated proxy last moved. To keep replication of the movement up to date while using
	//"OnRep_ReplicatedMovement()" thi
	float Time_Since_Last_Movement_Replication{};
	float Calculate_Speed();

	/**
	 * Player Health
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float Max_Health = 100.f;

	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(ReplicatedUsing = OnRep_Health, EditDefaultsOnly, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float Last_Health);

	/**
	 * Player Armor
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Player Stats")
	float Max_Armor = 100.f;

	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(ReplicatedUsing = OnRep_Armor, EditDefaultsOnly, Category = "Player Stats")
	float Armor = 100.f;

	UFUNCTION()
	void OnRep_Armor(float Last_Armor);

	UPROPERTY()
	AShooter_Player_Controller* Shooter_Player_Controller;

	bool bEliminated = false;

	FTimerHandle Eliminated_Timer;

	UPROPERTY(EditDefaultsOnly)
	float Eliminated_Delay = 3.f;

	void Eliminated_Timer_Finished();

	UPROPERTY()
	AShooter_Player_State* Shooter_Player_State;

	/**
	 * Grenade
	*/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Attached_Grenade;

public:	
	//Setter fucntion for setting the visibility of the Weapons "Pickup_Widget". Will be called in "AWeapon".
	void Set_Overlapping_Weapon(AWeapon* Weapon);
	
	//A getter function that returns true or false in result to a weapon being equipped.
	bool Is_Weapon_Equipped();
	
	//Return value is used in "UShooter_Anim_Instance"
	bool Is_Aiming();
	//Return value is used in "UShooter_Anim_Instance"
	bool Is_Sprinting();
	
	//Returns the "AO_Yaw" which is calculated in "&AShooter_Character::Aim_Offset". "UShooter_Anim_Instance" uses this value.
	FORCEINLINE float Get_AO_Yaw() const { return AO_Yaw;}
	//Returns the "AO_Pitch" which is calculated in "&AShooter_Character::Aim_Offset". "UShooter_Anim_Instance" uses this value.
	FORCEINLINE float Get_AO_Pitch() const { return AO_Pitch; }
	
	//Used by "Shooter_Anim_Instance" to obtain information about which weapon is equipped.
	AWeapon* Get_Equipped_Weapon();
	
	//Getter function for "UShooter_Anim_Instance to retrieve the state of Turning_In_Place"
	FORCEINLINE ETurning_In_Place Get_Turning_In_Place() const { return Turning_In_Place; }
	FVector Get_Hit_Target() const;
	FORCEINLINE UCameraComponent* Get_Follow_Camera() const { return Follow_Camera; }
	
	//Getter function for use within "UShooter_Anim_Instance" to determine whether or not the character should rotate the root bone or not.
	//Wheather it's a simulated proxy or "IsLocallyControlled()".
	FORCEINLINE bool Should_Rotate_Root_Bone() const { return bRotate_Root_Bone; }
	
	//Getter function for use within "UShooter_Anim_Instance" to determine whether the character has been eliminated (dead).
	FORCEINLINE bool Is_Eliminated() const { return bEliminated; }
	FORCEINLINE float Get_Health() const { return Health; }
	FORCEINLINE void Set_Health(float Amount) { Health = Amount;}
	FORCEINLINE float Get_Max_Health() const { return Max_Health; }
	FORCEINLINE float Get_Armor() const { return Armor;}
	FORCEINLINE void Set_Armor(float Amount) {Armor = Amount; }
	FORCEINLINE float Get_Max_Armor() const { return Max_Armor;}
	ECombat_State Get_Combat_State() const;
	FORCEINLINE UCombat_Component* Get_Combat() const { return Combat; }
	FORCEINLINE bool Get_Disable_Gameplay() const { return bDisable_Gameplay; }
	FORCEINLINE UAnimMontage* Get_Reload_Montage() const {return Reload_Montage;}
	FORCEINLINE UStaticMeshComponent* Get_Attached_Grenade() const { return Attached_Grenade; }
	FORCEINLINE UBuff_Component* Get_Buff() const { return Buff; }
};
