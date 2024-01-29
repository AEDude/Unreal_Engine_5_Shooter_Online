// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon_Types.h"
#include "Weapon.generated.h"

class UTexture2D;
class USoundCue;
class USkeletalMeshComponent;
class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;
class AShooter_Character;
class AShooter_Player_Controller;

/*Scoped enum class which holds all the states of the weapons.*/
//Blueprint type enum so it may be used in the editor.
UENUM(BlueprintType)	  //All enum constants will be unsigned 8bit integers.
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial_State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Equipped_Secondary UMETA(DisplayName = "Equipped_Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	//This enum constant returns the amount of enums in this enum class. 
	//This can be done by checking the numerical value of "EWS_MAX".
	EWS_MAX UMETA(DisplayName = "Default_Max")
};

UENUM(BlueprintType)
enum class EFire_Type : uint8
{
	EFT_Hit_Scan UMETA(DisplayName = "Hit_Scan_Weapon"),
	EFT_Projectile UMETA(DisplayName = "Projectile_Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotfun_Weapin"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class SHOOTER_ONLINE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*This function must be overriden on any class which replicates variables.*/
	//Register the variables which are to be replicated within the definition of this function.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void Set_HUD_Ammo();
	
	//defines wheather to show the pickup widget or hide it. 
	void Show_Pickup_Widget(bool bShow_Widget);
	virtual void Fire(const FVector& Hit_Target);
	void Drop_Weapons();
	void Add_Ammo(int32 Ammo_To_Add);
	FVector Trace_End_With_Scatter(const FVector& Hit_Target);

	/**
	* Textures for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* Crosshairs_Center;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* Crosshairs_Top;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* Crosshairs_Bottom;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* Crosshairs_Left;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* Crosshairs_Right;

	/**
	 * Zoomed Field of View while aiming
	*/

	UPROPERTY(EditAnywhere)
	float Zoomed_FOV = 33.f;

	UPROPERTY(EditAnywhere)
	float Zoom_Interp_Speed = 25.f;

	/**
	 * Automatic Firing Settings
	*/
	UPROPERTY(EditAnywhere, Category = "Combat")
	float Fire_Delay = .1f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomatic = true;

	UPROPERTY(EditAnywhere)
	USoundCue* Equip_Sound; 

	/**
	 * Eneble or disbale custom depth (color outline)
	*/
	void Eneble_Custom_Depth(bool bEneble);

	bool bDestroy_Weapon = false;

	UPROPERTY(EditDefaultsOnly)
	EFire_Type Fire_Type;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUse_Weapon_Scatter = false;

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void On_Weapon_State_Set();

	virtual void On_Equipped();

	virtual void On_Equipped_Secondary();

	virtual void On_Dropped();

	/*Only called on the server*/
	//The function which stores the logic for handling what happens when overlapping is active on the area sphere.
	//It is a virtual function so that the children of this class may override it.
	//Will be bound to the delagate "OnComponentBeginOverlap" so it needs to be a "UFUNCTION()". 
	UFUNCTION()
	virtual void On_Sphere_Overlap(
		UPrimitiveComponent* Overlapped_Component,
		AActor* Other_Actor,
		UPrimitiveComponent* Other_Comp,
		int32 Other_Body_Index,
		bool bFrom_Sweep,
		const FHitResult& Sweep_Result
		);
	/*Only called on the server*/
	//The function which stores the logic for handling what happens when overlapping ends on the area sphere.
	//It is not a virtual function because it should not be overriden.
	//Will be bound to the delagate "OnComponentEndOverlap" so it needs to be a "UFUNCTION()". 
	UFUNCTION()
	void On_Sphere_End_Overlap(
		UPrimitiveComponent* Overlapped_Component,
		AActor* Other_Actor,
		UPrimitiveComponent* Other_Comp,
		int32 Other_Body_Index
		);

	/**
	 * Trace End with scatter
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon_Scatter")
	float Distance_To_Sphere = 777.f;

	UPROPERTY(EditAnywhere, Category = "Weapon_Scatter")
	float Sphere_Radius = 70.f;

private:

	//Mesh type to initialize in the contructor. Has to be the same data type of the weapons which will be used (Skeletal Mesh). 
	UPROPERTY(VisibleAnywhere, Category = "Weapon_Properties")
	USkeletalMeshComponent* Weapon_Mesh;

	//Overlap volume which will eneble the player to pick up the weapon when overlap is occurring.
	UPROPERTY(VisibleAnywhere, Category = "Weapon_Properties")
	USphereComponent* Area_Sphere;

	//Variable of the enum class named "EWeaponState" This may be used similar to how a pointer is used.
	/*This pointer variable needs to be replicated so that when any change happens to the "AWeapon" class on the server, 
	(the server has authority over said class) said change will "replicate" to all clients.*/
	//Must use the "ReplicatedUsing =" specifier when replicating a variable using a Rep Notify.
	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()". 
	UPROPERTY(ReplicatedUsing = OnRep_Weapon_State, EditAnywhere, Category = "Weapon_Properties")
	EWeaponState Weapon_State;

	//Rep notify which will be called on the specific client that "Weapon_State" replicates to.
	//Rep notifies don't get called on the server. They only get called when the variable replicates. 
	//Replication only works one way from server to client. Therefore this rep notify will never be called on the server.
	//The only input parameter which may be passed into a rep notify is that of the same data type which is being replicated.
	//Said input parameter stores the last value of the variable which is being replicated before the most recent replication occurs.
	//Must be declared with a "UFUNCTION()" macro. 
	UFUNCTION()
	void OnRep_Weapon_State();
	
	//Pickup prompt will be displayed using this.
	UPROPERTY(EditAnywhere, Category = "Weapon_Properties")
	UWidgetComponent* Pickup_Widget;

	UPROPERTY(EditAnywhere, Category = "Weapon_Properties")
	UAnimationAsset* Fire_Animation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABullet_Casing> Bullet_Casing_Class;

	//Must register all replicated variables in the overidden function "GetLifetimeReplicatedProps()".
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UPROPERTY(EditAnywhere)
	int32 Magazine_Capacity;

	void Spend_Round();

	UFUNCTION()
	void OnRep_Ammo();

	UPROPERTY()
	AShooter_Character* Shooter_Owner_Character;

	UPROPERTY()
	AShooter_Player_Controller* Shooter_Owner_Controller;

	UPROPERTY(EditAnywhere)
	EWeapon_Type Weapon_Type;

public:	
	//Setter for the "EWeaponState" enum class. 
	void Set_Weapon_State(EWeaponState State);
	FORCEINLINE USphereComponent* Get_Area_Sphere() const { return Area_Sphere; }
	//"UShooter_Anim_Instance" uses this getter to transform the hands of the character in the appropriate locations.
	FORCEINLINE USkeletalMeshComponent* Get_Weapon_Mesh() const { return Weapon_Mesh; }
	FORCEINLINE float Get_Zoomed_FOV() const { return Zoomed_FOV; }
	FORCEINLINE float Get_Zoom_Interp_Speed() const { return Zoom_Interp_Speed; }
	bool Is_Empty();
	bool Is_Full();
	FORCEINLINE EWeapon_Type Get_Weapon_Type() const { return Weapon_Type; }
	FORCEINLINE int32 Get_Ammo() const { return Ammo; }
	FORCEINLINE int32 Get_Magazine_Capacity() const { return Magazine_Capacity; }

	};
