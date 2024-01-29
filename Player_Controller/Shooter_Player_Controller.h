// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Shooter_Player_Controller.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_ONLINE_API AShooter_Player_Controller : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	void Set_HUD_Health(float Health, float Max_Health);
	void Set_HUD_Armor(float Armor, float Max_Armor);
	void Set_HUD_Score(float Score);
	void Set_HUD_Deaths(int32 Deaths);
	void Set_HUD_Weapon_Ammo(int32 Ammo);
	void Set_HUD_Carried_Ammo(int32 Ammo);
	void Set_HUD_Match_Countdown(float Countdown_Time); 
	void Set_HUD_Announcement_Countdown(float Countdown_Time);
	void Set_HUD_Grenades(int32 Grenades);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;

	// Synced with Server World Clock
	float Get_Server_Time();
	
	//Sync with Server Clock as soon as possible
	virtual void ReceivedPlayer() override;
	
	void On_Match_State_Set(FName State);

	void Handle_Match_Started();

	void Handle_Cooldown();

protected:

	virtual void BeginPlay() override;
	void Set_HUD_Time();

	/**
	 * Sync time between client and sever
	*/

	// Requests the current server time passing in the clients time when the request was sent.
	UFUNCTION(Server, Reliable)
	void Server_Request_Server_Time(float Time_Of_Client_Request);

	//Reports the current Server Time to the client in response to Server Request Server Time.
	UFUNCTION(Client, Reliable)
	void Client_Report_Server_Time(float Time_Of_Client_Request, float Time_Server_Received_Client_Request);

	//Difference between Client and Server Time. 
	float Client_Server_Delta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Time")
	float Time_Sync_Frequency = 5.f;

	float Time_Sync_Running_Time = 0.f;

	void Check_Time_Sync(float DeltaTime);

	void Poll_Initialized();
	
	UFUNCTION(Server, Reliable)
	void Server_Check_Match_State();

	UFUNCTION(Client, Reliable)
	void Client_Join_Mid_Game(float Warmup, float Match, float Cooldown, float Starting_Time, FName State_Of_Match);


	void Check_Ping(float DeltaTime);
	void High_Ping_Warning();
	void Stop_High_Ping_Warning();

private:

	UPROPERTY()
	class AShooter_HUD* Shooter_HUD;

	UPROPERTY()
	class AShooter_Online_Game_Mode* Shooter_Online_Game_Mode;
	
	float Level_Starting_Time = 0.f;
	float Match_Time = 0.f;
	float Warmup_Time = 0.f;
	float Cooldown_Time = 0.f;
	uint32 Countdown_Integer = 0;

	UFUNCTION()
	void OnRep_Match_State();

	UPROPERTY(ReplicatedUsing = OnRep_Match_State)
	FName Match_State;

	UPROPERTY()
	class UCharacter_Overlay* Character_Overlay;

	bool bInitialized_Armor = false;
	bool bInitialized_Health = false;
	bool bInitialized_Score = false;
	bool bInitialized_Deaths = false;
	bool bInitialized_Grenades = false;
	bool bInitialize_Carried_Ammo = false;
	bool bInitialize_Weapon_Ammo = false;

	float HUD_Carried_Ammo;
	float HUD_Weapon_Ammo;
	float HUD_Armor = 0.f;
	float HUD_Max_Armor = 0.f;
	float HUD_Health = 0.f;
	float HUD_MAX_Health = 0.f;
	float HUD_Score = 0.f;
	int32 HUD_Deaths = 0;
	int32 HUD_Grenades = 0;

	float High_Ping_Running_Time = 0.f;
	
	UPROPERTY(EditDefaultsOnly)
	float High_Ping_Duration = 7.f;

	UPROPERTY(EditDefaultsOnly)
	float Ping_Animation_Running_Time = 0;

	UPROPERTY(EditDefaultsOnly)
	float Check_Ping_Frequency = 20.f;

	UPROPERTY(EditDefaultsOnly)
	float High_Ping_Threshold = 50.f;
	
	
};
