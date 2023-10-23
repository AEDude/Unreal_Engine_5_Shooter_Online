#pragma once

#define TRACE_LENGTH 70000

UENUM(BlueprintType)
enum class EWeapon_Type : uint8
{
    EWT_Pistol UMETA(DisplayName = "Pistol"),
    EWT_Submachine_Gun UMETA(DisplayName = "Submachine Gun"),
    EWT_Assult_Rifle UMETA(DisplayName = "Assult Rifle"),
    EWT_Shotgun UMETA(DisplayName = "Shotgun"),
    EWT_Sniper_Rifle UMETA(DisplayName = "Sniper Rifle"),
    EWT_Grenade_Launcher UMETA(DisplayName = "Grenade Launcher"),
    EWT_Rocket_Launcher UMETA(DisplayName = "Rocket Launcher"),

    EWT_MAX UMETA(DisplayName = "Default MAX")
};