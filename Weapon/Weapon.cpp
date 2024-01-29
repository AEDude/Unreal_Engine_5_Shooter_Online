// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Shooter_Online/Character/Shooter_Character.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h" 
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h" 
#include "Bullet_Casing.h"
#include "Shooter_Online/Player_Controller/Shooter_Player_Controller.h"
#include "Shooter_Online/Shooter_Components/Combat_Component.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	//Enebling replication so that the server has authority over all Weapon objects. 
	bReplicates = true;
	SetReplicateMovement(true);

	//Initialize the "Weapon_Mesh" && include Header.
	Weapon_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon_Mesh"));
	//Set the "Weapon_Mesh" as the root component of this class.
	SetRootComponent(Weapon_Mesh);

	//SetCollisionResponseToAllChannels on the "Weapon_Mesh" to block all so that the Weapon_Mesh collides everything when dropped.
	Weapon_Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//SetCollisionResponseToChannel on the "Weapon_Mesh" to ignore the pawn class so that characters don't collide with it when it is dropped. 
	Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//SetCollisionEnabled on the "Weapon_Mesh" to "NoCollision" so that the "Weapon_Mesh" starts of with no collision. Once it is dropped this will be changed.  
	Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Weapon_Mesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	Weapon_Mesh->MarkRenderStateDirty();
	Eneble_Custom_Depth(true);

	//Initialize the "Area_Sphere" && include Header.
	Area_Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Area_Sphere"));
	//Attach the "Area_Sphere" to the the "RootComponent" aka "Weapon_Mesh" as it's child.
	Area_Sphere->SetupAttachment(RootComponent);

	/*In a multiplayer game it's crucial to make sure that the server maintains authority over all important events, therefore 
	SetCollisionResponseToAllChannels for the "Area_Sphere" will be initialized with ignore. The server will take charge of
	allowing pawns to collide it, which will ultimately eneeble the pawn to pick up the weapon. */
	Area_Sphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//Initializing the collison for the "Area_Sphere" to "NoCollision". This will be changed on "Begin Play" but only on the server.
	Area_Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Initialize the "Pickup_Widget" && include Header
	Pickup_Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Pickup_Widget"));
	//Attach the "Pickup_Widget" to the "RootComponent" aka "Weapon_Mesh" as its child.
	Pickup_Widget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	//Setting the Collisions on the "Area_Sphere" to change from what they're initialized to in the contsructor.
	Area_Sphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Area_Sphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	
	//Binding the function "On_Sphere_Overlap()" to the delagate "OnComponentBeginOverlap".
	Area_Sphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::On_Sphere_Overlap);
	//Binding the function "On_Sphere_End_Overlap()" to the delagate "OnComponentBeginOverlap".
	Area_Sphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::On_Sphere_End_Overlap);
	
	if(Pickup_Widget)//Checking to see if "Pickup_Widget" is a null pointer.
	{
		//Setting the visibility of the pick up widget to false. This is changed by "AShooter_Character" 
		//when overlap events occour in the function "Set_Overlapping_Weapon()".
		Pickup_Widget->SetVisibility(false);
	}
	
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::Eneble_Custom_Depth(bool bEneble)
{
	if(Weapon_Mesh)
	{
		Weapon_Mesh->SetRenderCustomDepth(bEneble);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	//"Super" calls the original version of the function before it is overriden.
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	/*REGISTER THE VARIABLES WHICH ARE TO BE REPLCIATED*/ //Must include  #include "Net/UnrealNetwork.h"

			   //(Class which contains the replicated variable, Variable to Replicate).
	DOREPLIFETIME(AWeapon, Weapon_State);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::On_Sphere_Overlap(UPrimitiveComponent *Overlapped_Component, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, int32 Other_Body_Index, bool bFrom_Sweep, const FHitResult &Sweep_Result)
{	
	//This function is only called on the server.

	//Casting "Other_Actor" to AShooter_Character. Mush include the header file for "AShooter_Character".
	AShooter_Character* Shooter_Character = Cast<AShooter_Character>(Other_Actor);
	if(Shooter_Character)//If this is not a null pointer then the cast is valid and "Shooter_Character" has overlapped with the "Area_Sphere".
	{
		//Calling "Set_Overlapping_Weapon()" from AShooter_Character to set the visibility of the pickup widget. "Set_Overlapping_Weapon()"
		//stores a replicated variable so this will take care of replicating the status of the pickup widget on all clients.
		Shooter_Character->Set_Overlapping_Weapon(this);
	}
}

void AWeapon::On_Sphere_End_Overlap(UPrimitiveComponent *Overlapped_Component, AActor *Other_Actor, UPrimitiveComponent *Other_Comp, int32 Other_Body_Index)
{
	//This function is only called on the server.

	//Casting "Other_Actor" to AShooter_Character.
	AShooter_Character* Shooter_Character = Cast<AShooter_Character>(Other_Actor);
	if(Shooter_Character)
	{
											    //Setting this input parameter to nullptr because overlapping events have ended with the "Area_Sphere" and in result
												//"Overlapping_Weapon" on "AShooter_Character will be a nullptr."
		Shooter_Character->Set_Overlapping_Weapon(nullptr);
	}
}

void AWeapon::Set_HUD_Ammo()
{
	Shooter_Owner_Character = Shooter_Owner_Character == nullptr ? Cast<AShooter_Character>(GetOwner()) : Shooter_Owner_Character;
	if(Shooter_Owner_Character)
	{
		Shooter_Owner_Controller = Shooter_Owner_Controller == nullptr ? Cast<AShooter_Player_Controller>(Shooter_Owner_Character->Controller) : Shooter_Owner_Controller;
		if(Shooter_Owner_Controller)
		{
			Shooter_Owner_Controller->Set_HUD_Weapon_Ammo(Ammo);
		}
	}
}

void AWeapon::Spend_Round()
{
	Ammo = FMath::Clamp(Ammo -1, 0, Magazine_Capacity);
	Set_HUD_Ammo();
}

void AWeapon::OnRep_Ammo()
{
	Shooter_Owner_Character = Shooter_Owner_Character == nullptr ? Cast<AShooter_Character>(GetOwner()) : Shooter_Owner_Character;
	if(Shooter_Owner_Character && Shooter_Owner_Character->Get_Combat() && Is_Full())
	{
		Shooter_Owner_Character->Get_Combat()->Jump_To_Shotgun_End();
	}
	Set_HUD_Ammo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if(Owner == nullptr)
	{	
		Shooter_Owner_Character = nullptr;
		Shooter_Owner_Controller = nullptr;
	}
	else
	{	
		Shooter_Owner_Character = Shooter_Owner_Character == nullptr ? Cast<AShooter_Character>(Owner) : Shooter_Owner_Character;
		if(Shooter_Owner_Character && Shooter_Owner_Character->Get_Equipped_Weapon() && Shooter_Owner_Character->Get_Equipped_Weapon() == this)
		{
			Set_HUD_Ammo();
		}
	}
}

void AWeapon::Set_Weapon_State(EWeaponState State)
{
	/*"Set_Weapon_State" is only called on the server. The server has Authority of setting the 
	"Weapon_State" in "On_Weapon_State_Set()"*/

	//Setting the "Weapon_State" variable to "=" the input parameter which is passed in 
	//via "UCombat_Component" using syntax "EWeaponState::".
	/*Weapon_State is a replicated variable being replicated by "OnRep_Weapon_State()"*/
	//See "GetLifetimeReplicatedProps()".
	//This will result in "Weapon_State" replicating to all the clients via 
	//"OnRep_Weapon_State()".
	Weapon_State = State;
	
	//Calling "On_Weapon_State_Set()" on the server to handle what happens to the respective 
	//"AWeapon". This depends on the status being set on "EWeaponState::"
	On_Weapon_State_Set();
}

void AWeapon::On_Weapon_State_Set()
{
	switch (Weapon_State)
	{
	//If the weapon state is set to "EWS_Equipped" call "On_Equipped()".
	case EWeaponState::EWS_Equipped:
		On_Equipped();
		break;
	//If the weapon state is set to "EWS_Equipped_Secondary" call "On_Equipped_Secondary()".
	case EWeaponState::EWS_Equipped_Secondary:
		On_Equipped_Secondary();
		break;
	//If the weapon state is set to "EWS_Dropped" call "On_Dropped()"
	case EWeaponState::EWS_Dropped:
		On_Dropped();
		break;
	}
}

void AWeapon::OnRep_Weapon_State()
{
	/*"OnRep_Weapon_State()" is automatically called whenever "Weapon_State" changes in the function "Set_Weapon_State()".
	When "OnRep_Weapon_State()" is called "Weapon_State" replicates to all clients, setting the status of enum class "EWeaponState"*/
	//See "GetLifetimeReplicatedProps()". 

	//"On_Weapon_State_Set()" is called on all the clients, updating the "Weapon_State" of the respective "AWeapon" being replicated.
	On_Weapon_State_Set();
}

void AWeapon::On_Equipped()
{
	//Hide the "Pickup_Widget.".
	Show_Pickup_Widget(false);
	//Disbale the "Area_Sphere" so no overlap events can occur when the weapon is equipped.
	Area_Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon_Mesh->SetSimulatePhysics(false);
	Weapon_Mesh->SetEnableGravity(false);
	Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	Eneble_Custom_Depth(false);
}

void AWeapon::On_Equipped_Secondary()
{
	Show_Pickup_Widget(false);
	Area_Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon_Mesh->SetSimulatePhysics(false);
	Weapon_Mesh->SetEnableGravity(false);
	Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	Weapon_Mesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	Weapon_Mesh->MarkRenderStateDirty();
	Eneble_Custom_Depth(true);
}

void AWeapon::On_Dropped()
{
	if(HasAuthority())
		{
			Area_Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		Weapon_Mesh->SetSimulatePhysics(true);
		Weapon_Mesh->SetEnableGravity(true);
		Weapon_Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Weapon_Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		Weapon_Mesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		Weapon_Mesh->MarkRenderStateDirty();
}

void AWeapon::Show_Pickup_Widget(bool bShow_Widet)
{
	//Checking to see if the pickup widget is valid.
	if(Pickup_Widget)
	{
		//Setting the visibility of the pickup widget.
		Pickup_Widget->SetVisibility(bShow_Widet);
	}
}

void AWeapon::Fire(const FVector& Hit_Target)
{
	if(Fire_Animation)
	{
		Weapon_Mesh->PlayAnimation(Fire_Animation, false);
	}
	if(Bullet_Casing_Class)
	{
		const USkeletalMeshSocket* Ammo_Eject_Socket = Weapon_Mesh->GetSocketByName(FName("AmmoEject"));
    	if(Ammo_Eject_Socket)
    	{
        	FTransform Socket_Transform = Ammo_Eject_Socket->GetSocketTransform(Weapon_Mesh);

    		UWorld* World = GetWorld();
        	if(World)
        	{
            	World->SpawnActor<ABullet_Casing>(
            	Bullet_Casing_Class,
            	Socket_Transform.GetLocation(),
            	Socket_Transform.GetRotation().Rotator()
            	);
        	}
    	}
	}
	if(HasAuthority())
	{
		Spend_Round();
	}
}

void AWeapon::Drop_Weapons()
{
	Set_Weapon_State(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules Detach_Rules(EDetachmentRule::KeepWorld, true);
	Weapon_Mesh->DetachFromComponent(Detach_Rules);
	SetOwner(nullptr);
	Shooter_Owner_Character = nullptr;
	Shooter_Owner_Controller = nullptr;

}

void AWeapon::Add_Ammo(int32 Ammo_To_Add)
{
	Ammo = FMath::Clamp(Ammo - Ammo_To_Add, 0, Magazine_Capacity);
	Set_HUD_Ammo();
}

FVector AWeapon::Trace_End_With_Scatter(const FVector &Hit_Target)
{
    const USkeletalMeshSocket* Muzzle_Flash_Socket = Get_Weapon_Mesh()->GetSocketByName("MuzzleFlash");
    if(Muzzle_Flash_Socket == nullptr) return FVector();
    
    const FTransform Socket_Transform = Muzzle_Flash_Socket->GetSocketTransform(Get_Weapon_Mesh());
    const FVector Trace_Start = Socket_Transform.GetLocation();
    
    const FVector To_Target_Normalized = (Hit_Target - Trace_Start).GetSafeNormal();
    const FVector Sphere_Center = Trace_Start + To_Target_Normalized * Distance_To_Sphere;
    const FVector Random_Vector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, Sphere_Radius);
    const FVector End_Location = Sphere_Center + Random_Vector;
    const FVector To_End_Location = End_Location - Trace_Start;

    /*DrawDebugSphere(
        GetWorld(),
        Sphere_Center,
        Sphere_Radius,
        14,
        FColor::Yellow,
        true
    );

    DrawDebugSphere(
        GetWorld(),
        End_Location,
        5.f,
        14,
        FColor::Green,
        true
    );

    DrawDebugLine(GetWorld(), 
    Trace_Start,
    FVector (Trace_Start + To_End_Location * TRACE_LENGTH / To_End_Location.Size()),
    FColor::Red,
    true
    ); */
    return FVector (Trace_Start + To_End_Location * TRACE_LENGTH / To_End_Location.Size());
}

bool AWeapon::Is_Empty()
{
    return Ammo <= 0;
}

bool AWeapon::Is_Full()
{
	return Ammo == Magazine_Capacity;
}