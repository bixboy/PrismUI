#pragma once

#include "CoreMinimal.h"
#include "Components/PrismButtonBase.h"
#include "PrismExampleButton.generated.h"

/**
 * UPrismExampleButton
 * A concrete implementation of the Prism Button demonstrating:
 * 1. Role-based styling application.
 * 2. Visual state transitions (Normal -> Hovered -> Pressed).
 * 3. Tooltip and interaction best practices.
 */
UCLASS()
class PRISMUI_API UPrismExampleButton : public UPrismButtonBase
{
	GENERATED_BODY()

public:
	UPrismExampleButton(const FObjectInitializer& ObjectInitializer);

protected:
	// --- Lifecycle ---
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;

	// --- Style Overrides ---
	/** Called whenever a new style or theme is applied. */
	virtual void OnStyleApplied(const FPrismUIWidgetStyle& InStyleData) override;

	/** Called when the widget state changes (e.g., from Normal to Hovered). */
	virtual void OnStateChanged(EPrismWidgetState InNewState) override;

	/** Handles the visual tick for smooth transitions. */
	virtual void OnVisualsUpdated(float InBlendValue) override;

private:
	/** Internal helper to update text opacity based on state. */
	void UpdateTextVisuals(float InAlpha);

	UPROPERTY(EditAnywhere, Category = "Example | Visuals")
	float HoverTextOpacity = 0.8f;
};
