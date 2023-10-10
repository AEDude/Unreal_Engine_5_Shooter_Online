// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Shooter_Character.generated.h"

UCLASS()
class SHOOTER_ONLINE_API AShooter_Character : public ACharacter
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
	float AO_Pitch;
	FRotator Starting_Aim_Rotation;

public:	

	void Set_Overlapping_Weapon(AWeapon* Weapon);
	bool Is_Weapon_Equipped();
	bool Is_Aiming();
	FORCEINLINE float Get_AO_Yaw() const { return AO_Yaw;}
	FORCEINLINE float Get_AO_Pitch() const { return AO_Pitch; }
	AWeapon* Get_Equipped_Weapon();

};
