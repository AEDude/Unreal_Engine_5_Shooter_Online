#pragma once

/*Scoped enum class which holds all the states for turning in place.*/
//Blueprint type enum so it may be used in the editor.
UENUM(BlueprintType)	  //All enum constants will be unsigned 8bit integers.
enum class ETurning_In_Place : uint8
{
    ETIP_Left UMETA(DisplayName = "Turning_Left"),
    ETIP_Right UMETA(DisplayName = "Turning_Right"),
    ETIP_Not_Turning UMETA(DisplayName = "Not_Turning"),

    ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};