// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter_Character.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shooter_Online/Weapon//Weapon.h"
#include "Shooter_Online/Shooter_Components/Combat_Component.h"
#include "Shooter_Online/Shooter_Components/Buff_Component.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Shooter_Anim_Instance.h"
#include "Shooter_Online/Shooter_Online.h"
#include "Shooter_Online/Player_Controller/Shooter_Player_Controller.h"
#include "Shooter_Online/Game_Mode/Shooter_Online_Game_Mode.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Shooter_Online/Player_State/Shooter_Player_State.h"
#include "Shooter_Online/Weapon/Weapon_Types.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AShooter_Character::AShooter_Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Initialize the Camera_Boom && include Header.
	Camera_Boom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera_Boom"));
	//Attach Camera_Boom to character mesh because crouching changes capsule (RootComponent) size. Usually it would be attached to the RootComponent.
	Camera_Boom->SetupAttachment(GetMesh());
	//Set the Camera_Boom length.
	Camera_Boom->TargetArmLength = 555.f;
	//When disabled, the component will revert to using the stored RelativeRotation of the component.
	Camera_Boom->bUsePawnControlRotation = true;

	//Initialize Followe_Camera && include Header.
	Follow_Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow_Camera"));
	//Attach Follow_Camera to Camera_Boom.
	Follow_Camera->SetupAttachment(Camera_Boom, USpringArmComponent::SocketName);
	//Does not need to use the pawn control rotation because it's attached to the Camera_Boom.
	Follow_Camera->bUsePawnControlRotation = false;

	/*Don't forget to set this set the following two variables in the character Blueprint as well.*/
	// Inherited variable set to false so an Unequipped character doesn't rotate with the controller rotation.
	bUseControllerRotationYaw = false;
	//Set to true by accessing the Character Movement Component. This is so the character orients its direction to match with whichever WASD key is pressed.
	GetCharacterMovement()->bOrientRotationToMovement = true;

	//Initialize the Overhead_Widget which will display the players ID.
	Overhead_Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overhead_Widget"));
	//Attach the widget to the "RootComponent" of the character class.
	Overhead_Widget->SetupAttachment(RootComponent);

	//Initialize the "Combat_Component" && include Header.
	Combat = CreateDefaultSubobject<UCombat_Component>(TEXT("Combat_Component"));
	//Enable replication of the combat component so the variables on it which are replicated behave as expected. 
	/*Components don't need to be registered in "GetLifetimeReplicatedProps" they only need "SetIsReplicated" set to true.*/
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuff_Component>(TEXT("Buff_Component"));
	Buff->SetIsReplicated(true);

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	Attached_Grenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached_Grenade"));
	Attached_Grenade->SetupAttachment(GetMesh(), FName("Grenade_Socket"));
	Attached_Grenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Set the Capsule Component's collision to ignore the the camera so that the capsule doesn't block/ interact with said camera. 
	//If not done, the camera may snap in close when a capsule component intersects with the camera's (forward vector?). 
	//Include "#include "Components/CapsuleComponent.h""
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
	//Set the collision object tyoe for the mesh to "ECC_Skeletal_Mesh"
	GetMesh()->SetCollisionObjectType(ECC_Skeletal_Mesh);
	//Set the Mesh's's collision to ignore the the camera so that the camera doesn't snap in close/ interact with the character. 
	//If not done, the camera may snap in close when a the character intersects with the camera's (forward vector?).
	//Example: When backed up against a wall and the camera is behind the character. 
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	
	//"NavAgentProps.bCanCrouch" is set by Unreal Engine. It determines wheather the character can crouch or not. To bypass 
	//enebling "Can_Crouch" in "AShooter_Character" Character's Bluprint "NavAgentProps.bCanCrouch" is set to true here in the contstructor.
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	//Default value for enum class "ETurning_In_Place". The character should start off not turning.
	Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
	
	//How often the engine should replicate during max load of replication.
	NetUpdateFrequency = 70.f;
	//How often the engine should replicate when under low replication load.
	MinNetUpdateFrequency = 33.f;
}


// Called when the game starts or when spawned
void AShooter_Character::BeginPlay()
{
	Super::BeginPlay();

	Update_HUD_Health();
    Update_HUD_Armor();
	Update_HUD_Ammo();

	if(Combat)
	{
		Combat->Spawn_Default_Weapon();
	}

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AShooter_Character::Recieve_Damage);
	}

	if(Attached_Grenade)
	{
		Attached_Grenade->SetVisibility(false);
	}
}

// Called every frame
void AShooter_Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*Calling "Add_Input_Mapping_Context" in Tick to initialize the Server Controller. This is because the cast happens to early before the server gets initialized and in result 
	the cast returns a player controller there is a delay between the time the game launches */

	//"Add_Input_Mapping_Context" doesn't work when being called in Begin Play. This the case even when being called 7000x via a for loop.
	Add_Input_Mapping_Context(Shooter_Online_Mapping_Context, 0);
	
	Rotate_In_Place(DeltaTime);

	Hide_Camera_When_Character_Is_Close();
	Poll_Initialize();
}

// Called to bind functionality to input
void AShooter_Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Enhanced Input Action Binding
	if(UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(Jump_Action, ETriggerEvent::Triggered, this, &AShooter_Character::Jump);
		EnhancedInputComponent->BindAction(Move_Action, ETriggerEvent::Triggered, this, &AShooter_Character::Move);
		EnhancedInputComponent->BindAction(Look_Action, ETriggerEvent::Triggered, this, &AShooter_Character::Look);
		EnhancedInputComponent->BindAction(Equip_Action, ETriggerEvent::Triggered, this, &AShooter_Character::Equip_Button_Pressed);
		EnhancedInputComponent->BindAction(Crouch_Action, ETriggerEvent::Triggered, this, &AShooter_Character::Crouch_Button_Pressed);
		EnhancedInputComponent->BindAction(Aim_Action, ETriggerEvent::Started, this, &AShooter_Character::Aim_Button_Pressed);
		EnhancedInputComponent->BindAction(Aim_Action, ETriggerEvent::Completed, this, &AShooter_Character::Aim_Button_Released);
		EnhancedInputComponent->BindAction(Fire_Action, ETriggerEvent::Started, this, &AShooter_Character::Fire_Button_Pressed);
		EnhancedInputComponent->BindAction(Fire_Action, ETriggerEvent::Completed, this, &AShooter_Character::Fire_Button_Released);
		EnhancedInputComponent->BindAction(Reload_Action, ETriggerEvent::Triggered, this, &AShooter_Character::Reload_Button_Pressed);
		EnhancedInputComponent->BindAction(Grenade_Action, ETriggerEvent::Triggered, this, &AShooter_Character::Grenade_Button_Pressed);
		EnhancedInputComponent->BindAction(Sprint_Action, ETriggerEvent::Started, this, &AShooter_Character::Sprint_Button_Pressed);
		EnhancedInputComponent->BindAction(Sprint_Action, ETriggerEvent::Completed, this, &AShooter_Character::Sprint_Button_Released);
	}
	
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AShooter_Character::Add_Input_Mapping_Context(UInputMappingContext* Context_To_Add, int32 In_Priority)
{
	if(!Context_To_Add) return;
	
	//Get the player controller
	Shooter_Player_Controller = Cast<AShooter_Player_Controller>(GetController());
	if(Shooter_Player_Controller)
	{
			//Add Input Mapping Context
		if(Shooter_Player_Controller)
		{
			if(UEnhancedInputLocalPlayerSubsystem* Subsystem{ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(Shooter_Player_Controller->GetLocalPlayer())})
			{
				Subsystem->ClearAllMappings();
				Subsystem->AddMappingContext(Context_To_Add, In_Priority);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Input Mapping Initialization Failed"));
		}
	}
}	

void AShooter_Character::PostInitializeComponents()
{
	//"Super" calls the original version of the function before it is overriden.
	Super::PostInitializeComponents();

	//Just to be safe, before accessing the components listed below, it's good practice to always check if a pointer is valid (not null) 
	if(Combat)
	{
		//Initializing the AShooter_Character pointer variable within UCombat_Component to point to this class as early as possible.
		Combat->Character = this;
	}
	if(Buff)
	{
		//Initializing the AShooter_Character pointer variable within UBuff_Component to point to this class as early as possible.
		Buff->Character = this;
		Buff->Set_Initial_Speeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
	}
}

void AShooter_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//"Super" calls the original version of the function before it is overriden.
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*REGISTER THE VARIABLES WHICH ARE TO BE REPLCIATED*/ //Must include  #include "Net/UnrealNetwork.h"

				        //(Class which contains the replicated variable, Variable to Replicate, Condition of replication (COND_OwnerOnly only replicates to the owner of the pawn (this PC))).
	DOREPLIFETIME_CONDITION(AShooter_Character, Overlapping_Weapon, COND_OwnerOnly);
			     //(Class which contains the replicated variable, Variable to Replicate).
	DOREPLIFETIME(AShooter_Character, Health);
	DOREPLIFETIME(AShooter_Character, Armor);
	DOREPLIFETIME(AShooter_Character, bDisable_Gameplay);
	
}

void AShooter_Character::OnRep_ReplicatedMovement()
{
	/*This function is only for characters which are not locally contolled*/


	//"Super" calls the original version of the function before it is overriden.
	Super::OnRep_ReplicatedMovement();
	
	//Calling "Simulated_Proxies_Turn()" here instead of tick because for some reason when debugging while calling "Simulated_Proxies_Turn()"
	//in the tick fucntion the values of the Proxy_Yaw (the delta) kept resseting to "0" every one to two frames even though the controller was 
	//constantly rotating.
	//Calling "Simulated_Proxies_Turn() to see if ther is a change in the status of the simulated proxy, if there is OnRep_ReplicatedMovement()
	//will replicate the change."
	Simulated_Proxies_Turn();
	//Since replication just happened "Time_Since_Last_Movement_Replication" will be reset to "0". 
	Time_Since_Last_Movement_Replication = 0.f;
}

void AShooter_Character::Eliminated()
{
	Drop_Or_Destroy_Weapons();
	Multicast_Eliminated();
	GetWorldTimerManager().SetTimer(
		Eliminated_Timer,
		this,
		&AShooter_Character::Eliminated_Timer_Finished,
		Eliminated_Delay
	);
}

void AShooter_Character::Multicast_Eliminated_Implementation()
{	
	if(Shooter_Player_Controller)
	{
		Shooter_Player_Controller->Set_HUD_Weapon_Ammo(0);
	}
	bEliminated = true;
	Ragdoll_On_Death();
	//Play_Eliminated_Montage();

	//Disable All Controller Input
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if(Shooter_Player_Controller && Combat)
	{
		DisableInput(Shooter_Player_Controller);
		bDisable_Gameplay = true;
		Combat->Fire_Button_Pressed(false);
	}
	bool bHide_Sniper_Scope = IsLocallyControlled() && 
	Combat && 
	Combat->bAiming && 
	Combat->Equipped_Weapon && 
	Combat->Equipped_Weapon->Get_Weapon_Type() == EWeapon_Type::EWT_Sniper_Rifle;
	if(bHide_Sniper_Scope)
	{
		Show_Sniper_Scope_Widget(false);
	}
	
	/*Dsiable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);*/
}

void AShooter_Character::Eliminated_Timer_Finished()
{
	AShooter_Online_Game_Mode* Shooter_Online_Game_Mode = GetWorld()->GetAuthGameMode<AShooter_Online_Game_Mode>();
	if(Shooter_Online_Game_Mode)
	{
		Shooter_Online_Game_Mode->Request_Respawn(this, Controller);
	}
}

void AShooter_Character::Drop_Or_Destroy_Weapons()
{
	if(Combat)
	{
		if(Combat->Equipped_Weapon)
		{
			Drop_Or_Destroy_Weapon(Combat->Equipped_Weapon);
		}
		if(Combat->Secondary_Weapon)
		{
			Drop_Or_Destroy_Weapon(Combat->Secondary_Weapon);
		}
	}
}

void AShooter_Character::Drop_Or_Destroy_Weapon(AWeapon* Weapon)
{
	if(Weapon == nullptr) return;
	if(Weapon->bDestroy_Weapon)
	{
		Weapon->Destroy();
	}
	else
	{
		Weapon->Drop_Weapons();
	}
}

void AShooter_Character::Destroyed()
{
	Super::Destroyed();

	AShooter_Online_Game_Mode* Shooter_Game_Mode = Cast<AShooter_Online_Game_Mode>(UGameplayStatics::GetGameMode(this));
	bool bMatch_Is_not_In_Progress = Shooter_Game_Mode && Shooter_Game_Mode->GetMatchState() != MatchState::InProgress;
	if(Combat && Combat->Equipped_Weapon && bMatch_Is_not_In_Progress)
	{
		Combat->Equipped_Weapon->Destroy();
	}
}

void AShooter_Character::Play_Fire_Montage(bool bAiming)
{
	if(Combat == nullptr || Combat->Equipped_Weapon == nullptr) return;

	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Fire_Weapon_Montage)
	{
		Anim_Instance->Montage_Play(Fire_Weapon_Montage);
		FName Section_Name;
		Section_Name = bAiming ? FName("Rifle_Aim") : FName("Rifle_Hip");
		Anim_Instance->Montage_JumpToSection(Section_Name);
	}
}

void AShooter_Character::Play_Reload_Montage()
{
	if(Combat == nullptr || Combat->Equipped_Weapon == nullptr) return;

	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Reload_Montage)
	{
		Anim_Instance->Montage_Play(Reload_Montage);
		FName Section_Name;
		
		switch(Combat->Equipped_Weapon->Get_Weapon_Type())
		{
		case EWeapon_Type::EWT_Pistol:
			Section_Name = FName("Pistol");
			break;
		case EWeapon_Type::EWT_Submachine_Gun:
			Section_Name = FName("Submachine_Gun");
			break;
		case EWeapon_Type::EWT_Assult_Rifle:
			Section_Name = FName("Assult_Rifle");
			break;
		case EWeapon_Type::EWT_Shotgun:
			Section_Name = FName("Shotgun");
			break;
		case EWeapon_Type::EWT_Sniper_Rifle:
			Section_Name = FName("Rifle");
			break;
		case EWeapon_Type::EWT_Grenade_Launcher:
			Section_Name = FName("Assult_Rifle");
			break;	
		case EWeapon_Type::EWT_Rocket_Launcher:
			Section_Name = FName("Assult_Rifle");
			break;
		}

		Anim_Instance->Montage_JumpToSection(Section_Name);
	}
}

void AShooter_Character::Play_Hit_React_Montage()
{
	if(Combat == nullptr || Combat->Equipped_Weapon == nullptr) return;

	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Hit_React_Montage)
	{
		Anim_Instance->Montage_Play(Hit_React_Montage);
		FName Section_Name("From_Front");
		Anim_Instance->Montage_JumpToSection(Section_Name);
	}
}

void AShooter_Character:: Grenade_Button_Pressed()
{
	if(Combat)
	{
		Combat->Throw_Grenade();
	}
}

void AShooter_Character::Play_Eliminated_Montage()
{
	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Eliminated_Montage)
	{
		Anim_Instance->Montage_Play(Eliminated_Montage);
		/*FName Section_Name;
		Section_Name = bAiming ? FName("Rifle_Aim") : FName("Rifle_Hip");
		Anim_Instance->Montage_JumpToSection(Section_Name);*/
	}
}

void AShooter_Character::Play_Throw_Grenade_Montage()
{
	UAnimInstance* Anim_Instance = GetMesh()->GetAnimInstance();
	if(Anim_Instance && Throw_Grenade_Montage)
	{
		Anim_Instance->Montage_Play(Throw_Grenade_Montage);
	}
}

void AShooter_Character::Ragdoll_On_Death()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetSimulatePhysics(true);
}

void AShooter_Character::Recieve_Damage(AActor *Damaged_Actor, float Damage, const UDamageType *Damage_Type, AController *Instigator_Controller, AActor *Damage_Causer)
{
	if(bEliminated) return;

	float Damage_To_Health = Damage;
	if(Armor > 0.f)
	{
		if(Armor >= Damage)
		{
			Armor = FMath::Clamp(Armor - Damage, 0.f, Max_Armor);
			Damage_To_Health = 0.f;
		}
		else
		{
			Damage_To_Health = FMath::Clamp(Damage_To_Health - Armor, 0.f, Damage);
			Armor = 0.f;
		}
	}
	
	Health = FMath::Clamp(Health - Damage_To_Health, 0.f, Max_Health);
	
	Update_HUD_Health();
	Update_HUD_Armor();
	Play_Hit_React_Montage();
	if(Health == 0.f)
	{
		AShooter_Online_Game_Mode* Shooter_Online_Game_Mode = GetWorld()->GetAuthGameMode<AShooter_Online_Game_Mode>();
		if(Shooter_Online_Game_Mode)
		{	
			Shooter_Player_Controller = Shooter_Player_Controller == nullptr ? Cast<AShooter_Player_Controller>(Controller) : Shooter_Player_Controller;
			AShooter_Player_Controller* Attacker_Controller = Cast<AShooter_Player_Controller>(Instigator_Controller);
			Shooter_Online_Game_Mode->Player_Eliminated(this, Shooter_Player_Controller, Attacker_Controller);
		}
	}
	
}

void AShooter_Character::Move(const FInputActionValue& Value)
{
	if(bDisable_Gameplay) return;

	// Must convert from "const FInputActionValue& Value" input parameter to a Vector3D.
	const FVector3d Movement_Vector{Value.Get<FVector3d>()};

	if (Controller != nullptr)
	{
		/*Get the controllers forward direction not the characters forward vector.*/

		//Get the controllers rotation.
		const FRotator Rotation{Controller->GetControlRotation()};
		// Set which way is forward by initializing "X" and "Z" to "0" and setting "Y" (Unreal Engine's forward direction) to equal
		//Rotation.Yaw.
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get controller forward vector          
		const FVector ForwardDirection{FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)};
	
		// Get controllers right vector 
		const FVector RightDirection{FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y)};

		// Add movement 
		AddMovementInput(ForwardDirection, Movement_Vector.Y);
		AddMovementInput(RightDirection, Movement_Vector.X);
	}
	
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Move() failed within &AShooter_Character"));
	}
}

void AShooter_Character::Look(const FInputActionValue& Value)
{
	// Must convert from "const FInputActionValue& Value" input parameter to a Vector3D.
	FVector3d Look_Axis_Vector{Value.Get<FVector3d>()};

	if (Controller != nullptr)
	{
		// Add yaw and pitch input to controller
		AddControllerYawInput(Look_Axis_Vector.X);
		AddControllerPitchInput(-Look_Axis_Vector.Y);
	}
	
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Look() failed within &AShooter_Character"));
	}
}

void AShooter_Character::Equip_Button_Pressed()
{
	if(bDisable_Gameplay) return;
	//Making sure the "UCombat_Component" pointer is valid before moving forward.
	if(Combat)
	{
		//Calling "Server_Equip_Button_Pressed()" so the server can authorize and equip the weapon 
		//on the respective "AShooter_Character" which called the "Equip_Button_Pressed()" function. 
		Server_Equip_Button_Pressed();
	}
}

void AShooter_Character::Server_Equip_Button_Pressed_Implementation()
{
	/*In Unreal Engine, RPCs must have an "_Implementation" at the end of their definition name (only within the .cpp file). This is because Unreal Engine will 
	perform actions to modify the impementation of the definition so it behaves accordingly.*/
	
	//Making sure the "UCombat_Component" pointer is valid before accessing it.
	if(Combat)
	{
		//Making sure the "AWeapon" pointer Overlapping_Weapon is valid before accessing
		if(Overlapping_Weapon)
		{
			//Notifying "UCombat_Component" to "Equip_Weapon" with the "Overlapping_Weapon".
			Combat->Equip_Weapon(Overlapping_Weapon);
		}
		//Checking to see if the "AShooter_Character" has two "AWeapon" equipped. If this is the case call Swap_Weapons()
		//which is located in UCombat_Component.
		else if (Combat->Should_Swap_Weapons())
		{
			Combat->Swap_Weapons();
		}
	}
}

void AShooter_Character::Crouch_Button_Pressed()
{
	/*bIsCrouched && Crouch() && UnCrouch() are inherited from "&ACharacter". Replication is already set up by Unreal Engine*/
	
	if(!bIsCrouched)
	{
		Crouch();
	}
	else
	{
		UnCrouch();
	}
}

void AShooter_Character::Reload_Button_Pressed()
{	
	if(bDisable_Gameplay) return;

	if(Combat && !Combat->bSprinting)
	{
		Combat->Reload();
	}
}

void AShooter_Character::Aim_Button_Pressed()
{
	/*Called on the server only*/
	
	//Safety check
	if(Combat && !Combat->bSprinting && Get_Combat_State() != ECombat_State::ECS_Reloading)
	{	
		//Access "&UCombat_Component::Set_Aiming()" passing in true.
		//"Set_Aiming()" passes in the value of the input parameter which is set here, to the variable 
		//"&UCombat_Component::bAiming" within "&UCombat_Component::Set_Aiming().
		//"&UCombat_Component::bAiming" is retrieved from return bool fucntion "&AShooter_Character::Is_Aiming()"
		//Shooter_Anim_Instance access the return value of "&AShooter_Character::Is_Aiming()" to determine wheather or not to
		//change the animation pose.
		Combat->Set_Aiming(true);
	}
}

void AShooter_Character::Aim_Button_Released()
{	
	//Checking to see wheather or not pointer to "&UCombat_Component" is valid.
	if(Combat)
	{
		//Access "&UCombat_Component::Set_Aiming()" passing in false.
		//"Set_Aiming()" passes in the value of the input parameter which is set here, to the variable 
		//"&UCombat_Component::bAiming" within "&UCombat_Component::Set_Aiming().
		//"&UCombat_Component::bAiming" is retrieved from return bool fucntion "&AShooter_Character::Is_Aiming()"
		//Shooter_Anim_Instance access the return value of "&AShooter_Character::Is_Aiming()" to determine wheather or not to
		//change the animation pose.  
		Combat->Set_Aiming(false);
	}
}

bool AShooter_Character::Is_Aiming()
{
	/*See &AShooter_Character::Aim_Button_Pressed() && &AShooter_Character::Aim_Button_Released() */

	//Return value is used by "&Shooter_Anim_Instance" to determine whether or not "AShooter_Character" is aiming.
    return (Combat && Combat->bAiming);
}

float AShooter_Character::Calculate_Speed()
{
	FVector Velocity{GetVelocity()};
    Velocity.Z = 0.f;
    return Velocity.Size();
}

void AShooter_Character::Aim_Offset(float DeltaTime)
{	
	/*This function is only for locally contolled characters*/
	
	//Safety check
	if(Combat && Combat->Equipped_Weapon == nullptr || Combat->bSprinting == true) return;
    
	float Speed{Calculate_Speed()};
	bool bIs_In_Air{GetCharacterMovement()->IsFalling()};

	//Checking to see if the character is stationary and not jumping. When these two conditions are true
	//The charcter may adjust the AO_Yaw value within the Aim Offsets which in result will rotate the upper body while
	// the lower body remains facing its initial direction. Once the upper body reaches the threshold of rotation the lower
	//body will rotate to realign with the upper body. The "Current_Aim_Rotation" while "standing still and not jumping" 
	//will be stored using "GetBaseAimRotation().Yaw"
	if(Speed == 0.f && !bIs_In_Air) //Standing still and not jumping. 
	{
		//Enable "bRotate_Root_Bone" so that turning in place may happen once the threshold for rotating the upper body is met.
		bRotate_Root_Bone = true;
		//The controllers rotation Yaw while the character is standing still and not jumping.
		FRotator Current_Aim_Rotation{FRotator(0.f, GetBaseAimRotation().Yaw, 0.f)};
		//The delta between the  and the "Current_Aim_Rotation" which the controllers Yaw while standing still and the 
		//"Starting_Aim_Rotation" which is the default controller Yaw when the character is not running or jumping 
		//(facing directly ahead/ away from the camera).
		FRotator Delta_Aim_Rotation{UKismetMathLibrary::NormalizedDeltaRotator(Current_Aim_Rotation, Starting_Aim_Rotation)};
		//Assign the value of the "Delta_Aim_Rotation.Yaw" to AO_Yaw.
		AO_Yaw = Delta_Aim_Rotation.Yaw;
		
		
		//If the character is not "Turning_In_Place" "Interp_AO_Yaw" needs to be the same value as "AO_Yaw".
		//This is because when "Turning_In_Place" is activated "Interp_AO_Yaw" will interpolate to "0"
		//and "AO_Yaw will = Interp_AO_Yaw". In result so that the lower body will be able to turn in place 
		//and meet the rotation of the upper body while the upper body remains at it's current yaw rotation. 
		if(Turning_In_Place == ETurning_In_Place::ETIP_Not_Turning)
		{
			Interp_AO_Yaw = AO_Yaw;
		}
		//The root bone needs to remain still until "Turning_In_Place is activated, only then should the character turn in the appropriate
		//direction. Using a "Rotate Root Bone" node in ABP_Shooter to keep the root bone from rotating with the yaw of the controller."
		bUseControllerRotationYaw = true;

		//Turning in place.
		Turn_In_Place(DeltaTime);
	}
	
	//Checking to see if the character is running or jumping. When these two conditions are true
	//The charcter is facing directly away from the camera straight ahead with no offsets. This can be used to get the "GetBaseAimRotation().Yaw"
	//and store it as "Starting_Aim_Rotation". Ultimately the Aim_Offset.Yaw is the delta between "Starting_Aim_Rotation" and "Current_Aim_Rotation".
	if(Speed > 0.f || bIs_In_Air) // Running or jumping.
	{
		//Diable "bRotate_Root_Bone" so that turning in place does not occur when the character (bIs_Accelerating).
		bRotate_Root_Bone = false;
		//Controller rotation Yaw when the character (bIs_Accelerating)
		Starting_Aim_Rotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		//When the character is accelerating "AO_Yaw" should remian at "0". The goal is to keep the character's 
		//upper body from rotating while (bIs_Accelerating) or (bIs_In_Air).
		AO_Yaw = 0.f;
		//Enable "bUseControllerRotationYaw"
		bUseControllerRotationYaw = true;
		//Should not be "Turning_In_Place" if the character is (bIs_Accelerating) || (bIsFalling).
		Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
	}

	Calculate_AO_Pitch();
}

void AShooter_Character::Rotate_In_Place(float DeltaTime)
{
	//Only use Aim_Offset on the character that is AutonomousProxy, Authority and is Locally Controlled.
	if(GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		Aim_Offset(DeltaTime);
	}
	
	//If the character is a SimulatedProxy "OnRep_ReplicatedMovement()" will handle the different method of "Turning_In_Place".
	//For this to happen there needs to be a check to keep track of the time since the movement was last replicated. If this value
	//becomes greater than the set value "OnRep_ReplicatedMovement()" will be called and the timer (Time_Since_Last_Movement_Replication) 
	//will reset to "0" as defined in "&AShooter_Character::OnRep_ReplicatedMovement"
	else
	{
		Time_Since_Last_Movement_Replication += DeltaTime;
		if(Time_Since_Last_Movement_Replication > 0.3f)
		{
			OnRep_ReplicatedMovement();
		}
		
		//Calculate the AO_pitch for the simulated proxies every tick.
		Calculate_AO_Pitch();
	}
}

void AShooter_Character::Calculate_AO_Pitch()
{
	/*Unreal Engine serializes and compresses the pitch and the roll of the character via &CharacterMovementComponent::GetPackedAngles()
	when sending the values across the network. This means the values no longer go from -90 degrees to 90 degrees. This is because when 
	the the data is recieved on the receivers end of the network, the values are decompressed and in result the values are in a range of
	0 degrees to 360 degrees. (Once the pitch "goes beyond "0" towards the negative values due to compression it shoots up to 360 
	and begins to decrease until it raches 270 degrees).*/
	
	
	//Getting the pitch from the controllers rotation.
	AO_Pitch = GetBaseAimRotation().Pitch;

	//To fix the problem caused by the compression of the the pitch, a check to make sure the pitch is above 90 degrees &&
	//is not locally controlled (a simulated proxy) must be made
	if(AO_Pitch > 90.f && !IsLocallyControlled())
	{
		/*map pitch from the range [270, 360) to the range [-90, 0)*/
		
		//Current pitch will be in this range when viewing a character who is a simulated proxy.
		FVector2D In_Range(270.f, 360.f);
		//Range to fix the compression problem "In_Range"
		FVector2D Out_Range(-90.f, 0.f);
		//Apply the correction to the pitch of the sumulated proxy by using "FMath::GetMappedRangeValueClamped" 
		AO_Pitch = FMath::GetMappedRangeValueClamped(In_Range, Out_Range, AO_Pitch); //Check AO_Pitch and if it's in the "In_Range" 
		                                                                             //convert the values to the "Out_Range." 
	}
	
	//Used to fix pitch on server character
	/*if(!HasAuthority() && !IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AO Pitch %f"), AO_Pitch);
	}*/
}

void AShooter_Character::Turn_In_Place(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("AO_Yaw %f"), AO_Yaw);
	
	/*Determine wheather to turn in place in the direction of left or right*/

	//Turn right
	if(AO_Yaw > 90.f)
	{
		Turning_In_Place = ETurning_In_Place::ETIP_Right;
	}
	//Turn left
	else if (AO_Yaw < -90.f)
	{
		Turning_In_Place = ETurning_In_Place::ETIP_Left;
	}
	
	//Checing to see if the character is turning left or right.
	//If the character is "Turning_In_Place", "Interp_AO_Yaw" needs to interpolate to "0". While this is happening
	//"AO_Yaw will = Interp_AO_Yaw" and in result that the lower body will be able to turn in place and meet the rotation of the upper body.
	//while the upper body remains at it's current yaw rotation. 
	if(Turning_In_Place != ETurning_In_Place::ETIP_Not_Turning)
	{
		Interp_AO_Yaw = FMath::FInterpTo(Interp_AO_Yaw, 0.f, DeltaTime, 5.f);
		AO_Yaw = Interp_AO_Yaw;

		//Checking the absolute value of the "AO_Yaw" which is interpolating to "0" with "Interp_AO_Yaw" 
		//to see if the character has turned enough.
		if(FMath::Abs(AO_Yaw) < 8.f)
		{
			
			//If "AO_Yaw" which is interpolating to "0" with "Interp_AO_Yaw"
			//gets to any value below 8 degrees, set enum class "ETurning_In_Place"
			//to "Not_Turning" && reset the "Starting_Aim_Rotation" (see &AShooter_Character::Aim_Offset 4th "if" statement for explanation).
			Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
			Starting_Aim_Rotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); 
		}
	}
}

void AShooter_Character::Simulated_Proxies_Turn()
{
	/*This function is only for characters which are not locally contolled*/

	//Safety check
	if(Combat == nullptr || Combat->Equipped_Weapon == nullptr) return;
	//Disable the use of rotating the root bone. Simulated proxies will not depend on the use of this node.
	bRotate_Root_Bone = false;
	//If the simulated proxy (bIs_Accelerating) set enum class "ETurning_In_Place" to "Not_Turning" then return.
	float Speed{Calculate_Speed()};
	if(Speed > 0.f)
	{
		Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
		return;
	}

	//The value of "Proxy_Rotation_Last_Frame" will always be a little begind the value of "Proxy_Rotation".
    //This is because once "Proxy_Rotation" updates on the next tick, "Proxy_Rotation_Last_Frame" will not update until
    //the tick after that. Hence causing it to always be the previous value of "Proxy_Rotation" current value.
	Proxy_Rotation_Last_Frame = Proxy_Rotation;
	//The rotation of the "RootComponent"/Capsule Component
	Proxy_Rotation = GetActorRotation();
	//Use "NormalizedDeltaRotator" to calculate the difference between "Proxy_Rotation" && "Proxy_Rotation_Last_Frame".
	Proxy_Yaw = UKismetMathLibrary::NormalizedDeltaRotator(Proxy_Rotation, Proxy_Rotation_Last_Frame).Yaw;

	//UE_LOG(LogTemp, Warning, TEXT("Proxy Yaw %f"), Proxy_Yaw);

	//Checking to see if the absolute value of "Proxy_Yaw" is greater than the "Turn_Threshold" which is initialized to ".5f" (degrees). 
	if(FMath::Abs(Proxy_Yaw) > Turn_Threshold)
	{	
		//If "Proxy_Yaw" is greater than "Turn_Threshold" turn right.
		if(Proxy_Yaw > Turn_Threshold)
		{
			Turning_In_Place = ETurning_In_Place::ETIP_Right;
		}
		//If "Proxy_Yaw" is less than "-Turn_Threshold" turn left.
		else if (Proxy_Yaw < -Turn_Threshold)
		{
			Turning_In_Place = ETurning_In_Place::ETIP_Left;
		}
		//If neither are true then the simulated proxy is not turning.
		else
		{
			Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
		}
		return;
	}
	//If this line of code is reached, then the above if check was never entered. In result set enum class "ETurning_In_Place" to "ETIP_Not_Turning"
	Turning_In_Place = ETurning_In_Place::ETIP_Not_Turning;
}

bool AShooter_Character::Is_Sprinting()
{
	return (Combat && Combat->bSprinting);
}

void AShooter_Character::Sprint_Button_Pressed()
{
	if(bDisable_Gameplay) return;

	if(Combat && !Combat->bAiming && Combat->Get_Combat_State() != ECombat_State::ECS_Reloading)
	{
		if(bIsCrouched)
		{
			UnCrouch();
		}
		else
		{
			Combat->Set_Sprinting(true);
		}
	}
}

void AShooter_Character::Sprint_Button_Released()
{
	if(bDisable_Gameplay) return;

	Combat->Set_Sprinting(false);
}

void AShooter_Character::Jump()
{
	if(bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}

}

void AShooter_Character::Fire_Button_Pressed()
{
	if(bDisable_Gameplay) return;

	if(Combat && !Combat->bSprinting)
	{
		Combat->Fire_Button_Pressed(true);
	}
}

void AShooter_Character::Fire_Button_Released()
{
	if(bDisable_Gameplay) return;

	if(Combat)
	{
		Combat->Fire_Button_Pressed(false);
	}
}

void AShooter_Character::Hide_Camera_When_Character_Is_Close()
{
	if(!IsLocallyControlled()) return;
	/*if(((Follow_Camera->GetComponentLocation() - GetActorLocation()).Size()) < Camera_Threshold)
	{
		GetMesh()->SetVisibility(false);
		if(Combat && Combat->Equipped_Weapon->Get_Weapon_Mesh())
		{
			Combat->Equipped_Weapon->Get_Weapon_Mesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if(Combat && Combat->Equipped_Weapon->Get_Weapon_Mesh())
		{
			Combat->Equipped_Weapon->Get_Weapon_Mesh()->bOwnerNoSee = false;
		}
	}*/
}

void AShooter_Character::OnRep_Health(float Last_Health)
{
	Update_HUD_Health();
	if(Health < Last_Health)
	{
		Play_Hit_React_Montage();
	}
	
}

void AShooter_Character::OnRep_Armor(float Last_Armor)
{
	Update_HUD_Armor();
	if(Armor < Last_Armor)
	{
		//Play_Hit_React_Montage();
	}
}

void AShooter_Character::Update_HUD_Health()
{
	Shooter_Player_Controller = Shooter_Player_Controller == nullptr ? Cast<AShooter_Player_Controller>(Controller) : Shooter_Player_Controller;
	if(Shooter_Player_Controller)
	{
		Shooter_Player_Controller->Set_HUD_Health(Health, Max_Health);
	}
}

void AShooter_Character::Update_HUD_Armor()
{
	Shooter_Player_Controller = Shooter_Player_Controller == nullptr ? Cast<AShooter_Player_Controller>(Controller) : Shooter_Player_Controller;
	if(Shooter_Player_Controller)
	{
		Shooter_Player_Controller->Set_HUD_Armor(Armor, Max_Armor);
	}
}

void AShooter_Character::Update_HUD_Ammo()
{
	Shooter_Player_Controller = Shooter_Player_Controller == nullptr ? Cast<AShooter_Player_Controller>(Controller) : Shooter_Player_Controller;
	if(Shooter_Player_Controller && Combat && Combat->Equipped_Weapon)
	{
		Shooter_Player_Controller->Set_HUD_Carried_Ammo(Combat->Carried_Ammo);
		Shooter_Player_Controller->Set_HUD_Weapon_Ammo(Combat->Equipped_Weapon->Get_Ammo());
	}
}

void AShooter_Character::Poll_Initialize()
{
	if(Shooter_Player_State == nullptr)
	{
		Shooter_Player_State = GetPlayerState<AShooter_Player_State>();
		if(Shooter_Player_State)
		{
			Shooter_Player_State->Add_To_Score(0.f);
			Shooter_Player_State->Add_To_Deaths(0);
		}
	}
}

void AShooter_Character::Set_Overlapping_Weapon(AWeapon* Weapon)
{
	/*ONLY CALLED ON THE SERVER*/

	//Taking care of displaying the "Pickup_Widget" on the Server if overlapping events are occurring.
	//The rep notify "OnRep_Overlapping_Weapon()" will never be called on the server because replication only happens on the
	//clients. Therefore the logic in this function will only be called on the server. 

	//Making sure "Overlapping_Weapon" isn't a null pointer. 
	if(Overlapping_Weapon != nullptr)
	{
		//Setting the default visibility to false before setting the value of "Overlapping_Weapon" = "Weapon". 
		Overlapping_Weapon->Show_Pickup_Widget(false);
	}

	Overlapping_Weapon = Weapon;

	//Checking to see if the server is controlling the "AShooter_Character" pawn that is causeing overlapping events to occur.
	if(IsLocallyControlled())
	{	
		//Making sure "Overlapping_Weapon" isn't a null pointer.
		if(Overlapping_Weapon != nullptr)
		{
			Overlapping_Weapon->Show_Pickup_Widget(true);
		}
	}
}

void AShooter_Character::OnRep_Overlapping_Weapon(AWeapon* Last_Weapon)
{
	/*ONLY CALLED ON CLIENTS*/

	//Taking care of displaying the pickup widget on the client which "Overlapping_Weapon" replicates to. 

	//Making sure "Overlapping_Weapon" isn't a null pointer. This is because the server may have set it to be null and in
	//result that change will replicate to all clients. 
	if(Overlapping_Weapon != nullptr)
	{
		Overlapping_Weapon->Show_Pickup_Widget(true);
	}

	//Do to the fact that "Last_Weapon" stores the last state of the variable which is being replicated aka "Overlapping_Weapon" 
	//before the most recent replication occured, the visibility of the "Pickup_Widget" may be set to false once overlapping events
	//end in "AWeapon::On_Sphere_End_Overlap" because "Last_Weapon" will still have the valid data. 

	if(Last_Weapon != nullptr)
	{
		Last_Weapon->Show_Pickup_Widget(false);
	}
}

bool AShooter_Character::Is_Weapon_Equipped()
{
	//Check to see if the pointer to the "UCombat_Component" component class is valid within this class.
	//Check to see if the "AWeapon" pointer "Equipped_Weapon" which is within the "UCombat_Component" is valid.
	
	/*Equipped_Weapon" which an "AWeapon" pointer gets information passed into it via the variable "Weapon_To_Equip" (an input parameter 
	at &Combat_Component::. "Weapon_To_Equip" gets information passed into it via the overlapping events which happen at "&AWeapon::On_Sphere_Overlap".
	"&AWeapon::On_Sphere_Overlap" passes in the weapon which is in close proximity of the "AShooter_Character" to this class via "&AWeapon::On_Sphere_Overlap". 
	&AShooter_Character::Set_Overlapping_Weapon receives the information and passes said information to the "AWeapon" pointer "Overlapping_Weapon". The visibilty of the 
	"Pickup_Widget is then enebled or disabled accordingly when the "AShooter_Character" picks up or drops the weapon*/

	//Therefore,
	//Check to see if the pointer to the "UCombat_Component" component class is valid within this class.
	//Check to see if the "AWeapon" pointer "Equipped_Weapon" which is within the "UCombat_Component" is valid. If it is valid a weapon is equipped.
	//Shooter_Anim_Instance uses this return value.
    
	return (Combat && Combat->Equipped_Weapon);
}

AWeapon *AShooter_Character::Get_Equipped_Weapon()
{
	/*"UShooter_Anim_Instance" uses this getter function to retrieve the weapon which is equipped*/


	//Check to see if the pointer to "UCombat_Component" is valid before accessing it". 
	//If it returns null pointer, return nullptr:
   if(Combat == nullptr) return nullptr;
   	//If the pointer to "UCombat_Component" is valid return the "Equipped_Weapon"
	//There is no way for the pointer to "UCombat_Component" to be valid without an "Equipped_Weapon" being valid. 
	return Combat->Equipped_Weapon;
}

FVector AShooter_Character::Get_Hit_Target() const
{
    if(Combat == nullptr) return FVector();
	return Combat->Hit_Target;
}

ECombat_State AShooter_Character::Get_Combat_State() const
{
     if(Combat == nullptr) return ECombat_State::ECS_MAX;
	return Combat->Combat_State;
}