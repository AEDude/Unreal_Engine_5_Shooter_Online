// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat_Component.h"
#include "Shooter_Online\Weapon\Weapon.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DefaultPawn.h"
#include "DrawDebugHelpers.h"
#include "Shooter_Online/Player_Controller/Shooter_Player_Controller.h"
//#include "Shooter_Online/HUD/Shooter_HUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

// Sets default values for this component's properties
UCombat_Component::UCombat_Component()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	
	PrimaryComponentTick.bCanEverTick = true;

	Base_Walk_Speed = 450.f;
	Aim_Walk_Speed = 300.f;
	Sprint_Speed = 800.f;

	// ...
}

void UCombat_Component::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UCombat_Component, Equipped_Weapon);
	DOREPLIFETIME(UCombat_Component, bAiming);
	DOREPLIFETIME(UCombat_Component, bSprinting);
	DOREPLIFETIME_CONDITION(UCombat_Component, Carried_Ammo, COND_OwnerOnly);
	DOREPLIFETIME(UCombat_Component, Combat_State);
}

// Called when the game starts
void UCombat_Component::BeginPlay()
{
	Super::BeginPlay();

	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Base_Walk_Speed;

		if(Character->Get_Follow_Camera())
		{
			Default_FOV = Character->Get_Follow_Camera()->FieldOfView;
			Current_FOV = Default_FOV;
		}
		if(Character->HasAuthority())
		{
			Initialize_Carried_Ammo();
		}
	}
	// ...
}

// Called every frame
void UCombat_Component::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(Character && Character->IsLocallyControlled())
	{
		FHitResult Hit_Result; 
		Trace_Under_Crosshairs(Hit_Result);
		Hit_Target = Hit_Result.ImpactPoint;

		Set_HUD_Crosshairs(DeltaTime);
		Interp_FOV(DeltaTime);
	}
	// ...
}

void UCombat_Component::Set_HUD_Crosshairs(float DeltaTime)
{
	if(Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<AShooter_Player_Controller>(Character->Controller) : Controller;
	if(Controller)
	{
		HUD = HUD == nullptr ? Cast<AShooter_HUD>(Controller->GetHUD()) : HUD;
		if(HUD)
		{	
			if(Equipped_Weapon)
			{
				HUD_Package.Crosshair_Center = Equipped_Weapon->Crosshairs_Center;
				HUD_Package.Crosshair_Top = Equipped_Weapon->Crosshairs_Top;
				HUD_Package.Crosshair_Bottom = Equipped_Weapon->Crosshairs_Bottom;
				HUD_Package.Crosshair_Left = Equipped_Weapon->Crosshairs_Left;
				HUD_Package.Crosshair_Right = Equipped_Weapon->Crosshairs_Right;
			}
			else
			{
				HUD_Package.Crosshair_Center = nullptr;
				HUD_Package.Crosshair_Top = nullptr;
				HUD_Package.Crosshair_Bottom = nullptr;
				HUD_Package.Crosshair_Left = nullptr;
				HUD_Package.Crosshair_Right = nullptr;
			}
			//Calculate crosshair spread

			//Walk Speed [0, 450] -> [0, 1]
			FVector2D Walk_Speed_Range(0.f, Base_Walk_Speed);
			FVector2D Velocity_Multiplier_Range(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			
			Crosshair_Velocity_Factor = FMath::GetMappedRangeValueClamped(Walk_Speed_Range, Velocity_Multiplier_Range, Velocity.Size());

			if(Character->GetCharacterMovement()->IsFalling())
			{
				Crosshair_In_Air_Factor = FMath::FInterpTo(Crosshair_In_Air_Factor, 2.5f, DeltaTime, 2.5f);
			}
			else
			{
				Crosshair_In_Air_Factor = FMath::FInterpTo(Crosshair_In_Air_Factor, 0.f, DeltaTime, 40.f);
			}
			
			if(bAiming)
			{
				Crosshair_Aim_Factor = FMath::FInterpTo(Crosshair_Aim_Factor, -.7f, DeltaTime, 33.f);
			}
			else
			{
				Crosshair_Aim_Factor = FMath::FInterpTo(Crosshair_Aim_Factor, 0.f, DeltaTime, 33.f);
			}

			Crosshair_Firing_Weapon_Factor = FMath::FInterpTo(Crosshair_Firing_Weapon_Factor, 0.f, DeltaTime, 44.f);
			
			HUD_Package.Crosshair_Spread = 
			0.7f +
			Crosshair_Velocity_Factor + 
			Crosshair_In_Air_Factor +
			Crosshair_Aim_Factor +
			Crosshair_Firing_Weapon_Factor;

			HUD->Set_HUD_Package(HUD_Package);
		} 
	}
}

void UCombat_Component::Interp_FOV(float DeltaTime)
{
	if(Equipped_Weapon == nullptr) return;

	if(bAiming)
	{
		Current_FOV = FMath::FInterpTo(Current_FOV, Equipped_Weapon->Get_Zoomed_FOV(), DeltaTime, Equipped_Weapon->Get_Zoom_Interp_Speed());
	}
	else
	{
		Current_FOV = FMath::FInterpTo(Current_FOV, Default_FOV, DeltaTime, Equipped_Weapon->Zoom_Interp_Speed);
	}
	if(Character && Character->Get_Follow_Camera())
	{
		Character->Get_Follow_Camera()->SetFieldOfView(Current_FOV);
	}
}

void UCombat_Component::Set_Aiming(bool bIs_Aiming)
{
	if(Character == nullptr || Equipped_Weapon == nullptr) return;
	bAiming = bIs_Aiming;
	Server_Set_Aiming(bIs_Aiming);
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIs_Aiming ? Aim_Walk_Speed : Base_Walk_Speed;
	}
	if(Character->IsLocallyControlled() && Equipped_Weapon->Get_Weapon_Type() == EWeapon_Type::EWT_Sniper_Rifle)
	{
		Character->Show_Sniper_Scope_Widget(bIs_Aiming);
	}
}

void UCombat_Component::Server_Set_Aiming_Implementation(bool bIs_Aiming)
{
	bAiming = bIs_Aiming;
	if(Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIs_Aiming ? Aim_Walk_Speed : Base_Walk_Speed;
	}
}

void UCombat_Component::Sprint_Button_Pressed(bool bPressed)
{
	bSprint_Button_Pressed = bPressed;
	if(Character && bSprint_Button_Pressed)
	{
		if(Equipped_Weapon == nullptr)
		{
			Start_Sprinting_Unequipped(bSprint_Button_Pressed);
		}
		else
		{
			Start_Sprinting_Equipped(bSprint_Button_Pressed);
		}
	}
	
	if(Character && !bSprint_Button_Pressed)
	{
		if(Equipped_Weapon == nullptr)
		{
			Stop_Sprinting_Unequipped(bSprint_Button_Pressed);
		}
		else
		{
			Stop_Sprinting_Equipped(bSprint_Button_Pressed);
		}
	}
}

void UCombat_Component::Start_Sprinting_Unequipped(bool bIs_Sprinting)
{
	bSprinting = bIs_Sprinting;
	Server_Start_Sprinting_Unequipped(bIs_Sprinting);
	if(bSprinting && Sprint_Speed)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Sprint_Speed;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = false;
	}
}

void UCombat_Component::Stop_Sprinting_Unequipped(bool bIs_Sprinting)
{
	bSprinting = bIs_Sprinting;
	Server_Stop_Sprinting_Unequipped(bIs_Sprinting);
	Character->GetCharacterMovement()->MaxWalkSpeed = Base_Walk_Speed;
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	Character->bUseControllerRotationYaw = false;
}

void UCombat_Component::Start_Sprinting_Equipped(bool bIs_Sprinting)
{	
	bSprinting = bIs_Sprinting;
	Server_Start_Sprinting_Equipped(bIs_Sprinting);
	if(bSprinting && Sprint_Speed)
	{	
		Character->GetCharacterMovement()->MaxWalkSpeed = Sprint_Speed;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = false;
	}
}

void UCombat_Component::Stop_Sprinting_Equipped(bool bIs_Sprinting)
{
		bSprinting = bIs_Sprinting;
		Server_Stop_Sprinting_Equipped(bIs_Sprinting);
		Character->GetCharacterMovement()->MaxWalkSpeed = Base_Walk_Speed;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
}

void UCombat_Component::Server_Start_Sprinting_Unequipped_Implementation(bool bIs_Sprinting)
{
	bSprinting = bIs_Sprinting;
	if(bSprinting && Sprint_Speed)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = Sprint_Speed;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = false;
	}
}

void UCombat_Component::Server_Start_Sprinting_Equipped_Implementation(bool bIs_Sprinting)
{
	bSprinting = bIs_Sprinting;
	if(bSprinting && Sprint_Speed)
	{	
		Character->GetCharacterMovement()->MaxWalkSpeed = Sprint_Speed;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
		Character->bUseControllerRotationYaw = false;
	}
}

void UCombat_Component::Server_Stop_Sprinting_Unequipped_Implementation(bool bIs_Sprinting)
{
	bSprinting = bIs_Sprinting;
	Character->GetCharacterMovement()->MaxWalkSpeed = Base_Walk_Speed;
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	Character->bUseControllerRotationYaw = false;
}

void UCombat_Component::Server_Stop_Sprinting_Equipped_Implementation(bool bIs_Sprinting)
{
	bSprinting = bIs_Sprinting;
	Character->GetCharacterMovement()->MaxWalkSpeed = Base_Walk_Speed;
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombat_Component::Fire_Button_Pressed(bool bPressed)
{
	bFire_Button_Pressed = bPressed;
	if(Character && bFire_Button_Pressed)
	{
		if(Combat_State != ECombat_State::ECS_Reloading)
		{
			Fire();
		}
	}
}

void UCombat_Component::Fire()
{
	if(Can_Fire())
	{
		/*FHitResult Hit_Result;
		Trace_Under_Crosshairs(Hit_Result);*/
		Server_Fire(Hit_Target);
		if(Equipped_Weapon)
		{
			Crosshair_Firing_Weapon_Factor = .7f;
			bCan_Fire = false;
		}
		Start_Fire_Timer();
	}
}

void UCombat_Component::Start_Fire_Timer()
{
	if(Equipped_Weapon == nullptr || Character == nullptr) return;
		Character->GetWorldTimerManager().SetTimer(
		Fire_Timer,
		this,
		&UCombat_Component::Fire_Timer_Finished,
		Equipped_Weapon->Fire_Delay
	);
}

void UCombat_Component::Fire_Timer_Finished()
{
	if(Equipped_Weapon == nullptr) return;
	bCan_Fire = true;
	if(bFire_Button_Pressed && Equipped_Weapon->bAutomatic)
	{	
		Fire();
	}
	
	if(Equipped_Weapon->Is_Empty())
	{
		Reload();
	}
}

void UCombat_Component::Server_Fire_Implementation(const FVector_NetQuantize& Trace_Hit_Target)
{
	Multicast_Fire(Trace_Hit_Target);
}

void UCombat_Component::Multicast_Fire_Implementation(const FVector_NetQuantize& Trace_Hit_Target)
{
	if(Equipped_Weapon == nullptr) return;
	if(Character && Combat_State == ECombat_State::ECS_Unoccupied)
	{
		Character->Play_Fire_Montage(bAiming);
		Equipped_Weapon->Fire(Trace_Hit_Target);
	}
}

void UCombat_Component::Trace_Under_Crosshairs(FHitResult& Trace_Hit_Result)
{
	FVector2D Viewport_Size;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(Viewport_Size);
	}

	FVector2D Crosshair_Location(Viewport_Size.X / 2.f, Viewport_Size.Y / 2.f);
	FVector Crosshair_World_Position;
	FVector Crosshair_World_Direction;
	bool bScreen_To_World = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		Crosshair_Location,
		Crosshair_World_Position,
		Crosshair_World_Direction
	);

	if(bScreen_To_World)
	{
		FVector Start = Crosshair_World_Position;
		
		if(Character)
		{
			float Distance_To_Character = (Character->GetActorLocation() - Start).Size();
			Start += Crosshair_World_Direction * (Distance_To_Character + 55.f);
			//DrawDebugSphere(GetWorld(), Start, 17.f, 15, FColor::Emerald);
		}


		FVector End = Start + Crosshair_World_Direction * TRACE_LENGTH;
		GetWorld()->LineTraceSingleByChannel(Trace_Hit_Result, 
		Start, 
		End, 
		ECollisionChannel::ECC_Visibility
		);
		if(!Trace_Hit_Result.bBlockingHit) Trace_Hit_Result.ImpactPoint = End;

		if(Trace_Hit_Result.GetActor() && Trace_Hit_Result.GetActor()->Implements<UCrosshairs_Interaction_Interface>())
		{
			HUD_Package.Crosshairs_Color = FLinearColor::Red;
		}
		else
		{
			HUD_Package.Crosshairs_Color = FLinearColor::White;
		}
		
		/*if(!Trace_Hit_Result.bBlockingHit)
		{
			Trace_Hit_Result.ImpactPoint = End;
			Hit_Target = End;
		}
		else
		{
			Hit_Target = Trace_Hit_Result.ImpactPoint;
			DrawDebugSphere(
				GetWorld(),
				Trace_Hit_Result.ImpactPoint,
				15.f,
				15.f,
				FColor::Blue
				);
		}*/
	}
}

void UCombat_Component::Equip_Weapon(AWeapon* Weapon_To_Equip)
{
	if(Character == nullptr || Weapon_To_Equip == nullptr || bSprinting) return;
	if(Equipped_Weapon)
	{
		Equipped_Weapon->Drop_Weapons();
	}
	
	Equipped_Weapon = Weapon_To_Equip;
	Equipped_Weapon->Set_Weapon_State(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* Hand_Socket = Character->GetMesh()->GetSocketByName(FName("Right_Hand_Socket"));
	if(Hand_Socket)
	{
		Hand_Socket->AttachActor(Equipped_Weapon, Character->GetMesh());
	}
	
	Equipped_Weapon->SetOwner(Character);
	Equipped_Weapon->Set_HUD_Ammo();
	
	if(Carried_Ammo_Map.Contains(Equipped_Weapon->Get_Weapon_Type()))
	{
		Carried_Ammo = Carried_Ammo_Map[Equipped_Weapon->Get_Weapon_Type()];
	}
	
	Controller = Controller == nullptr ? Cast<AShooter_Player_Controller>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->Set_HUD_Carried_Ammo(Carried_Ammo);
	}

	if(Equipped_Weapon->Equip_Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			Equipped_Weapon->Equip_Sound,
			Character->GetActorLocation()
		);
	}

	if(Equipped_Weapon->Is_Empty())
	{
		Reload();
	}

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombat_Component::Reload()
{
	if(Carried_Ammo > 0 && Combat_State != ECombat_State::ECS_Reloading)
	{
		Server_Reload();
	}
}

void UCombat_Component::Server_Reload_Implementation()
{
	if(Character == nullptr || Equipped_Weapon == nullptr || Amount_To_Reload() == 0) return;

	Combat_State = ECombat_State::ECS_Reloading;
	Handle_Reload();
}

void UCombat_Component::Finish_Reloading()
{
	if(Character == nullptr) return;
	if(Character->HasAuthority())
	{
		Combat_State = ECombat_State::ECS_Unoccupied;
		Update_Ammo_Values();
	}
	if(bFire_Button_Pressed)
	{
		Fire();
	}
}

void UCombat_Component::Update_Ammo_Values()
{
	if(Character == nullptr || Equipped_Weapon == nullptr) return;
	int32 Reload_Amount = Amount_To_Reload();
	if(Carried_Ammo_Map.Contains(Equipped_Weapon->Get_Weapon_Type()))
	{
		Carried_Ammo_Map[Equipped_Weapon->Get_Weapon_Type()] -= Reload_Amount;
		Carried_Ammo = Carried_Ammo_Map[Equipped_Weapon->Get_Weapon_Type()];
	}

	Controller = Controller == nullptr ? Cast<AShooter_Player_Controller>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->Set_HUD_Carried_Ammo(Carried_Ammo);
	}
	Equipped_Weapon->Add_Ammo(-Reload_Amount);
}

void UCombat_Component::OnRep_Combat_State()
{
	switch(Combat_State)
	{
	case ECombat_State::ECS_Reloading:
		Handle_Reload();
		break;
	case ECombat_State::ECS_Unoccupied:
		if(bFire_Button_Pressed)
		{
			Fire();
		}
		break;
	}
}

void UCombat_Component::Handle_Reload()
{
	Character->Play_Reload_Montage();
}

int32 UCombat_Component::Amount_To_Reload()
{	
	if(Equipped_Weapon == nullptr) return 0;
	int32 Room_In_Magazine = Equipped_Weapon->Get_Magazine_Capacity() - Equipped_Weapon->Get_Ammo();
	
	if(Carried_Ammo_Map.Contains(Equipped_Weapon->Get_Weapon_Type()))
	{
		int32 Amount_Of_Ammo_Carried = Carried_Ammo_Map[Equipped_Weapon->Get_Weapon_Type()];
		int32 Least = FMath::Min(Room_In_Magazine, Amount_Of_Ammo_Carried);
		return FMath::Clamp(Room_In_Magazine, 0, Least);
	}
    return 0;
}

void UCombat_Component::OnRep_Equipped_Weapon()
{
	if(Equipped_Weapon && Character)
	{
		Equipped_Weapon->Set_Weapon_State(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* Hand_Socket = Character->GetMesh()->GetSocketByName(FName("Right_Hand_Socket"));
		if(Hand_Socket)
		{
			Hand_Socket->AttachActor(Equipped_Weapon, Character->GetMesh());
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		if(Equipped_Weapon->Equip_Sound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				Equipped_Weapon->Equip_Sound,
				Character->GetActorLocation()
			);
		}
	}
}

bool UCombat_Component::Can_Fire()
{
    if(Equipped_Weapon == nullptr) return false;
	return !Equipped_Weapon->Is_Empty() && bCan_Fire && Combat_State == ECombat_State::ECS_Unoccupied;
}

void UCombat_Component::OnRep_Carried_Ammo()
{
	Controller = Controller == nullptr ? Cast<AShooter_Player_Controller>(Character->Controller) : Controller;
	if(Controller)
	{
		Controller->Set_HUD_Carried_Ammo(Carried_Ammo);
	}
}

void UCombat_Component::Initialize_Carried_Ammo()
{
	Carried_Ammo_Map.Emplace(EWeapon_Type::EWT_Pistol, Starting_Pistol_Ammo);
	Carried_Ammo_Map.Emplace(EWeapon_Type::EWT_Submachine_Gun, Starting_Submachine_Gun_Ammo);
	Carried_Ammo_Map.Emplace(EWeapon_Type::EWT_Assult_Rifle, Starting_Assult_Rifle_Ammo);
	Carried_Ammo_Map.Emplace(EWeapon_Type::EWT_Shotgun, Starting_Shotgun_Ammo);
	Carried_Ammo_Map.Emplace(EWeapon_Type::EWT_Sniper_Rifle, Starting_Sniper_Rifle_Ammo);
	Carried_Ammo_Map.Emplace(EWeapon_Type::EWT_Grenade_Launcher, Starting_Grenade_Launcher_Ammo);
	Carried_Ammo_Map.Emplace(EWeapon_Type::EWT_Rocket_Launcher, Starting_Rocket_Launcher_Ammo);
}
