#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styles/PrismUIStyle.h"
#include "PrismWidgetBase.generated.h"

class UPrismUIThemeData;


/**
 * UPrismWidgetBase — The foundational base for all Prism UI widgets.
 * Handles automatic role-based styling and theme reactivity.
 * Zero-tick by default.
 */
UCLASS(Abstract)
class PRISMUI_API UPrismWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UPrismWidgetBase(const FObjectInitializer& ObjectInitializer);

	// --- Theme Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Theme")
	FName WidgetRole = "Primary";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Theme")
	TObjectPtr<UPrismUIStyle> StyleOverride = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Theme")
	void RefreshStyle();

	virtual UPrismWidgetBase* SetRole(FName InRole);

	virtual UPrismWidgetBase* SetStyleOverride(UPrismUIStyle* InStyle);

	// --- State Management ---
	UFUNCTION(BlueprintCallable, Category = "Prism UI | State")
	void SetWidgetState(EPrismWidgetState InNewState);

	UFUNCTION(BlueprintPure, Category = "Prism UI | State")
	FORCEINLINE EPrismWidgetState GetCurrentState() const { return CurrentState; }

protected:
	
	// --- Native ---
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	// --- Style ---
	/** Returns the specific style data to be applied. Resolves: StyleOverride -> RoleMapping -> Null */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Theme")
	const FPrismUIWidgetStyle& GetEffectiveStyle() const;

	/** Builds the core C++ UI hierarchy. Override in child classes for programmatic UI. */
	virtual void BuildDefaultLayout();

	virtual void OnStyleApplied(const FPrismUIWidgetStyle& InStyleData);
	
	// --- Animation ---
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Visuals")
	void StartVisualTransition(float InTargetValue, float InSpeed = 15.0f);
	
	/** Starts the unified transition timer if it is not already running. */
	void RequestTransitionTick();

	/** 
	 * Custom transition logic evaluated on the timer interval. 
	 * @return True if transitions are still animating, False to sleep the timer.
	 */
	virtual bool TickTransitions(float DeltaTime);

	virtual void OnVisualsUpdated(float InBlendValue);
	
	virtual void OnStateChanged(EPrismWidgetState InNewState);

	/** Safely plays a UMG animation if it exists. */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Visuals")
	void PlayUMGAnimationSafe(UWidgetAnimation* InAnim, float InStartAtTime = 0.0f, int32 InNumLoopsToPlay = 1, EUMGSequencePlayMode::Type InPlayMode = EUMGSequencePlayMode::Forward, float InPlaybackSpeed = 1.0f);

protected:
	/** Centralized timer for all visual transitions. */
	FTimerHandle TransitionTimer;

	/** Centralized blended alpha for visuals (0.0 to 1.0). Accessible by children. */
	float CurrentVisualAlpha = 0.0f;

	/** Target blended alpha for visuals. */
	float TargetVisualAlpha = 0.0f;

	/** Speed of the transition. */
	float TransitionSpeed = 15.0f;

private:
	UFUNCTION()
	void HandleThemeChanged(UPrismUIThemeData* InNewTheme);

	void InternalTransitionTick();

	static const FPrismUIWidgetStyle& GetDefaultFallbackStyle();

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | State")
	EPrismWidgetState CurrentState = EPrismWidgetState::Normal;

	/** Dirty flag for style updates. */
	bool bIsAttributesDirty = true;
};
