// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial_State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
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
	void Show_Pickup_Widget(bool bShow_Widget);
	virtual void Fire(const FVector& Hit_Target);

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

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

public:	

	void Set_Weapon_State(EWeaponState State);
	FORCEINLINE USphereComponent* Get_Area_Sphere() const { return Area_Sphere; }
	FORCEINLINE USkeletalMeshComponent* Get_Weapon_Mesh() const { return Weapon_Mesh; }
	FORCEINLINE float Get_Zoomed_FOV() const { return Zoomed_FOV; }
	FORCEINLINE float Get_Zoom_Interp_Speed() const { return Zoom_Interp_Speed; }

};
