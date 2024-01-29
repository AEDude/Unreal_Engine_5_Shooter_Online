#pragma once

/*Scoped enum class which holds all the states for the character handling the weapons.*/
//Blueprint type enum so it may be used in the editor.
UENUM(BlueprintType)	  //All enum constants will be unsigned 8bit integers.
enum class ECombat_State : uint8
{
    ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
    ECS_Reloading UMETA(DisplayName = "Reloading"),
    ECS_Throwing_Grenade UMETA(DisplayName = "Throwing_Grenade"),

    ECS_MAX UMETA(DisplayName = "Default MAX")
};