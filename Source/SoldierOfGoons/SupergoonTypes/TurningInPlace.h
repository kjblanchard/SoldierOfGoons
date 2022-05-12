#pragma once

//Allows us to use this enum in Unreal engine as a blueprint type
UENUM(BlueprintType)
enum class ETurningInPlace: uint8
{
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	ETIP_NotTurning UMETA(DisplayName = "Turning Nowhere"),
	ETIP_MAX UMETA(DisplayName = "Max"),
	
};
