// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	void Set_Aiming(bool bIs_Aiming);

	UFUNCTION(Server, Reliable)
	void Server_Set_Aiming(bool bIs_Aiming);

	UFUNCTION()
	void OnRep_Equipped_Weapon();

private:
	class AShooter_Character* Character;
	
	UPROPERTY(ReplicatedUsing = OnRep_Equipped_Weapon)
	AWeapon* Equipped_Weapon;

	UPROPERTY(Replicated)
	bool bAiming;
	
	UPROPERTY(EditAnywhere)
	float Base_Walk_Speed;
	
	UPROPERTY(EditAnywhere)
	float Aim_Walk_Speed;

public:	

		
};
