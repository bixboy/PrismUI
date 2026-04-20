// Copyright Prism UI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PrismUITypes.h"
#include "PrismUIStyle.generated.h"

/**
 * UPrismUIStyle — Master visual configuration asset.
 * Defines a complete set of design tokens (Colors, Fonts, Brushes, Sounds)
 * that represent a specific visual aesthetic.
 */
UCLASS(BlueprintType, Category = "Prism UI")
class PRISMUI_API UPrismUIStyle : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Global configuration for this style. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Design Tokens")
	FPrismUIWidgetStyle StyleData;
};
