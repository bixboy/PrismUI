#pragma once
#include "CoreMinimal.h"
#include "Components/PrismToggleBase.h"
#include "PrismToggleSwitch.generated.h"

class UBorder;
class UOverlay;
class USizeBox;


UCLASS()
class PRISMUI_API UPrismToggleSwitch : public UPrismToggleBase
{
	GENERATED_BODY()

public:
	UPrismToggleSwitch(const FObjectInitializer& ObjectInitializer);
	
	virtual void SetIsChecked(bool bInIsChecked, bool bInBroadcast = true) override;

protected:
	
	virtual void OnStyleApplied(const FPrismUIWidgetStyle& InStyleData) override;

	virtual void BuildDefaultLayout() override;

	virtual void OnVisualsUpdated(float InBlendValue) override;

	virtual bool TickTransitions(float DeltaTime) override;

protected:
	/** Animation Alpha for the Current State (Hover/Press). */
	float InteractiveAlpha = 0.0f;
	float TargetInteractiveAlpha = 0.0f;
private:
	void UpdateThumbPosition(float InAlpha);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<USizeBox> RootSizer = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UOverlay> RootOverlay = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UBorder> TrackBorder = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UBorder> ThumbBorder = nullptr;

	UPROPERTY(EditAnywhere, Category = "Prism UI | Animation")
	float AnimationSpeed = 15.0f;
};
