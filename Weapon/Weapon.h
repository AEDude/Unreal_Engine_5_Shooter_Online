// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon_Types.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial_State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Equipped_Secondary UMETA(DisplayName = "Equipped_Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "Default_Max")
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void Set_HUD_Ammo();
	void Show_Pickup_Widget(bool bShow_Widget);
	virtual void Fire(const FVector& Hit_Target);
	void Drop_Weapons();
	void Add_Ammo(int32 Ammo_To_Add);

	/**
	* Textures for the weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* Crosshairs_Center;

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
	class USoundCue* Equip_Sound; 

	/**
	 * Eneble or disbale custom depth (color outline)
	*/
	void Eneble_Custom_Depth(bool bEneble);

	bool bDestroy_Weapon = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void On_Weapon_State_Set();

	virtual void On_Equipped();

	virtual void On_Equipped_Secondary();

	virtual void On_Dropped();

	UFUNCTION()
	virtual void On_Sphere_Overlap(
		UPrimitiveComponent* Overlapped_Component,
		AActor* Other_Actor,
		UPrimitiveComponent* Other_Comp,
		int32 Other_Body_Index,
		bool bFrom_Sweep,
		const FHitResult& Sweep_Result
	);

	UFUNCTION()
	void On_Sphere_End_Overlap(
		UPrimitiveComponent* Overlapped_Component,
		AActor* Other_Actor,
		UPrimitiveComponent* Other_Comp,
		int32 Other_Body_Index
	);

private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USkeletalMeshComponent* Weapon_Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* Area_Sphere;

	UPROPERTY(ReplicatedUsing = OnRep_Weapon_State, EditAnywhere, Category = "Weapon_Properties")
	EWeaponState Weapon_State;

	UFUNCTION()
	void OnRep_Weapon_State();
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* Pickup_Widget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* Fire_Animation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ABullet_Casing> Bullet_Casing_Class;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 Ammo;

	UPROPERTY(EditAnywhere)
	int32 Magazine_Capacity;

	void Spend_Round();

	UFUNCTION()
	void OnRep_Ammo();

	UPROPERTY()
	class AShooter_Character* Shooter_Owner_Character;

	UPROPERTY()
	class AShooter_Player_Controller* Shooter_Owner_Controller;

	UPROPERTY(EditAnywhere)
	EWeapon_Type Weapon_Type;

public:	

	void Set_Weapon_State(EWeaponState State);
	FORCEINLINE USphereComponent* Get_Area_Sphere() const { return Area_Sphere; }
	FORCEINLINE USkeletalMeshComponent* Get_Weapon_Mesh() const { return Weapon_Mesh; }
	FORCEINLINE float Get_Zoomed_FOV() const { return Zoomed_FOV; }
	FORCEINLINE float Get_Zoom_Interp_Speed() const { return Zoom_Interp_Speed; }
	bool Is_Empty();
	bool Is_Full();
	FORCEINLINE EWeapon_Type Get_Weapon_Type() const { return Weapon_Type; }
	FORCEINLINE int32 Get_Ammo() const { return Ammo; }
	FORCEINLINE int32 Get_Magazine_Capacity() const { return Magazine_Capacity; }

	};
