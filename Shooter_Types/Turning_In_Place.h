#pragma once

UENUM(BlueprintType)
enum class ETurning_In_Place : uint8
{
    ETIP_Left UMETA(DisplayName = "Turning_Left"),
    ETIP_Right UMETA(DisplayName = "Turning_Right"),
    ETIP_Not_Turning UMETA(DisplayName = "Not_Turning"),

    ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};