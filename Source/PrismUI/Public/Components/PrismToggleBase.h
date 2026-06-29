#pragma once
#include "CoreMinimal.h"
#include "Components/PrismButtonBase.h"
#include "PrismToggleBase.generated.h"

class UPrismToggleBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPrismOnToggleChanged, UPrismToggleBase*, Toggle, bool, bNewState);


UCLASS(Abstract)
class PRISMUI_API UPrismToggleBase : public UPrismButtonBase
{
	GENERATED_BODY()

public:
	// --- State Control ---

	UFUNCTION(BlueprintPure, Category = "Prism UI | Toggle")
	bool IsChecked() const { return bIsChecked; }

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Toggle")
	virtual void SetIsChecked(bool bInIsChecked, bool bInBroadcast = true);

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Events")
	FPrismOnToggleChanged OnToggleChanged;

protected:
	virtual void NativeDestruct() override;

	virtual void HandleClick() override;

	virtual bool TickTransitions(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Prism UI | Toggle")
	bool bIsChecked = false;

	// Animation
	
	float ToggleAlpha = 0.00f;
	
	float TargetToggleAlpha = 0.00f;
};
