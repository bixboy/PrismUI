#pragma once
#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismSliderBase.generated.h"

class UPrismSliderBase;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPrismOnSliderChanged, UPrismSliderBase*, Slider, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPrismOnSliderSend, UPrismSliderBase*, Slider, float, NewValue);


/**
 * UPrismSliderBase — Generic logic for slider components.
 * Manages value ranges and normalized alpha for visual representation.
 */
UCLASS(Abstract)
class PRISMUI_API UPrismSliderBase : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	// --- Range Control ---

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Slider")
	void SetValueRange(float InMin, float InMax);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Slider")
	void SetValue(float InValue, bool bInBroadcast = true);

	UFUNCTION(BlueprintPure, Category = "Prism UI | Slider")
	float GetValue() const { return CurrentValue; }
	
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Slider")
	void SendCurrentValue();

	UFUNCTION(BlueprintPure, Category = "Prism UI | Slider")
	float GetNormalizedValue() const;

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Events")
	FPrismOnSliderChanged OnValueChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Events")
	FPrismOnSliderSend OnValueSend;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void BuildDefaultLayout() override;

    virtual void OnVisualsUpdated(float InBlendValue) override;

	UPROPERTY(EditAnywhere, Category = "Prism UI | Slider")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Prism UI | Slider")
	float MaxValue = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Prism UI | Slider")
	float CurrentValue = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "Prism UI | Slider")
	float StepSize = 0.0f;

	
    /** Root container. */
	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<class UOverlay> RootOverlay = nullptr;

    /** The interactive slider component. */
	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<class USlider> InternalSlider = nullptr;

    /** The visual progress bar (fill). */
	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<class UProgressBar> FillBar = nullptr;

private:
    UFUNCTION()
    void HandleOnSliderValueChanged(float InNewValue);
};
