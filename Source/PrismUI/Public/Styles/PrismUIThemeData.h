// Copyright Prism UI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PrismUIThemeData.generated.h"

class UPrismUIStyle;

/**
 * UPrismUIThemeData — Mapping asset for role-based styling.
 * Associates Widget Roles (FName) with specific PrismUIStyle assets.
 */
UCLASS(BlueprintType, Category = "Prism UI")
class PRISMUI_API UPrismUIThemeData : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Maps a Widget Role (e.g., "Primary", "Danger", "Transparent") to a specific Style asset. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme Configuration")
	TMap<FName, TObjectPtr<UPrismUIStyle>> RoleMapping;

	/** Resolves a style for a given role. Returns null if not found. */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Theme")
	UPrismUIStyle* GetStyleForRole(FName InRole) const
	{
		const TObjectPtr<UPrismUIStyle>* FoundStyle = RoleMapping.Find(InRole);
		return FoundStyle ? *FoundStyle : nullptr;
	}
};
