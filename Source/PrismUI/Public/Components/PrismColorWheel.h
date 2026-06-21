// Copyright (c) Bixboy, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismColorWheel.generated.h"

class UImage;
class USpacer;
class UOverlay;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPrismOnColorChanged, FLinearColor, NewColor);

/**
 * UPrismColorWheel — A premium circular color picker.
 * Allows selecting Hue and Saturation in a single 2D space.
 */
UCLASS()
class PRISMUI_API UPrismColorWheel : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UPrismColorWheel(const FObjectInitializer& ObjectInitializer);

	/** Event triggered during selection (real-time) */
	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Events")
	FPrismOnColorChanged OnColorChanged;

	/** Event triggered when the mouse is released after selection */
	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Events")
	FPrismOnColorChanged OnColorCommitted;

	/** Sets the current color and updates the selector position */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Color")
	void SetSelectedColor(FLinearColor InColor, bool bBroadcast = true);

	/** Returns the current selected color (Saturation and Hue) */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Color")
	FLinearColor GetSelectedColor() const { return SelectedColor; }

protected:
	virtual void NativeConstruct() override;
	virtual void BuildDefaultLayout() override;

	// --- Input ---
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void OnVisualsUpdated(float InBlendValue) override;

private:
	void ProcessMouseInput(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);
	void UpdateSelectorPosition();

	UPROPERTY(Transient)
	FLinearColor SelectedColor = FLinearColor::Red;

	UPROPERTY(Transient)
	bool bIsDragging = false;

	// --- UI Components ---
	
	UPROPERTY()
	TObjectPtr<UOverlay> RootContainer = nullptr;

	/** The background wheel (should be a Material with Hue/Sat circular gradient) */
	UPROPERTY()
	TObjectPtr<UImage> Image_Wheel = nullptr;

	/** The selection handle */
	UPROPERTY()
	TObjectPtr<UImage> Image_Selector = nullptr;

	/** Spacer to maintain aspect ratio 1:1 */
	UPROPERTY()
	TObjectPtr<USpacer> AspectSpacer = nullptr;
};
