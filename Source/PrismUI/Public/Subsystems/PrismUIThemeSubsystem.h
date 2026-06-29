#pragma once
#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Styles/PrismUIThemeData.h"
#include "PrismUIThemeSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrismThemeChanged, UPrismUIThemeData*, NewTheme);

/**
 * UPrismUIThemeSubsystem — Central engine for global UI theming.
 * Tracks the current active theme and notifies widgets when the theme changes.
 */
UCLASS()
class PRISMUI_API UPrismUIThemeSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	// --- Lifecycle ---

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Theme Management ---

	/** Sets the global active theme and notifies all listeners. */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Theme")
	void SetActiveTheme(UPrismUIThemeData* InTheme);

	/** Returns the currently active global theme. */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Theme")
	UPrismUIThemeData* GetActiveTheme() const { return ActiveTheme; }

	/** Resolves a style for a given role from the active theme. */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Theme")
	UPrismUIStyle* GetStyleForRole(FName InRole) const;

	/** Resolves a specific color token from the active theme for a given role. */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Theme")
	FLinearColor GetColorToken(FName InRole, EPrismColorToken InToken) const;

	/** Resolves a specific typography token from the active theme for a given role. */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Theme")
	FSlateFontInfo GetTypographyToken(FName InRole, EPrismTypographyToken InToken) const;

	// --- Events ---

	/** Fired when the global theme is changed. */
	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Theme")
	FOnPrismThemeChanged OnThemeChanged;

private:
	/** The current active UI theme. */
	UPROPERTY()
	TObjectPtr<UPrismUIThemeData> ActiveTheme = nullptr;
};
