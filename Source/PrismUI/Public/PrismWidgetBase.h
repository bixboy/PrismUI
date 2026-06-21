// Copyright (c) Bixboy, 2026. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styles/PrismUIStyle.h"
#include "Interfaces/PrismPoolableWidget.h"
#include "PrismWidgetBase.generated.h"

class UPrismUIThemeData;


UCLASS(Abstract)
class PRISMUI_API UPrismWidgetBase : public UUserWidget, public IPrismPoolableWidget
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

	UFUNCTION(BlueprintPure, Category = "Prism UI | 3D Interaction")
	bool IsRenderedIn3D() const { return GetOwningWidgetComponent() != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Prism UI | 3D Interaction")
	UWidgetComponent* GetOwningWidgetComponent() const;

	UFUNCTION(BlueprintPure, Category = "Prism UI | 3D Interaction")
	FVector2D GetInteractionMovementDelta(const FPointerEvent& InMouseEvent) const;


	// --- Pooling ---

	virtual void OnAcquiredFromPool_Implementation() override;
	virtual void OnReleasedToPool_Implementation() override;


	// --- Data Binding ---

	void AddBindingCleanup(TFunction<void()> CleanupFunc);
	void ClearAllBindings();

protected:
	
	// --- Native ---

	virtual bool Initialize() override;
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	

	// --- Style ---

	UFUNCTION(BlueprintPure, Category = "Prism UI | Theme")
	const FPrismUIWidgetStyle& GetEffectiveStyle() const;

	virtual void BuildDefaultLayout();

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Visuals")
	void InitializeLayout();

	virtual void OnStyleApplied(const FPrismUIWidgetStyle& InStyleData);
	

	// --- Animation ---

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Visuals")
	void StartVisualTransition(float InTargetValue, float InSpeed = 15.0f);
	
	void RequestTransitionTick();


	virtual bool TickTransitions(float DeltaTime);

	virtual void OnVisualsUpdated(float InBlendValue);
	
	virtual void OnStateChanged(EPrismWidgetState InNewState);


	UFUNCTION(BlueprintCallable, Category = "Prism UI | Visuals")
	void PlayUMGAnimationSafe(UWidgetAnimation* InAnim, float InStartAtTime = 0.0f, int32 InNumLoopsToPlay = 1, 
		EUMGSequencePlayMode::Type InPlayMode = EUMGSequencePlayMode::Forward, float InPlaybackSpeed = 1.0f);


protected:
	
	TArray<TFunction<void()>> ActiveBindings;

	FTimerHandle TransitionTimer;

	float CurrentVisualAlpha = 0.0f;

	float TargetVisualAlpha = 0.0f;

	float TransitionSpeed = 15.0f;


private:
	UFUNCTION()
	void HandleThemeChanged(UPrismUIThemeData* InNewTheme);

	void InternalTransitionTick();

	static const FPrismUIWidgetStyle& GetDefaultFallbackStyle();

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | State")
	EPrismWidgetState CurrentState = EPrismWidgetState::Normal;

	UPROPERTY()
	mutable TWeakObjectPtr<UWidgetComponent> CachedOwningWidgetComponent;

	bool bIsAttributesDirty = true;

	float LastTickTime = -1.0f;

	UPROPERTY(Transient)
	bool bHasBuiltLayout = false;
};
