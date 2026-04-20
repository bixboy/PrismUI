#pragma once
#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismProgressBarBase.generated.h"


UCLASS(Abstract)
class PRISMUI_API UPrismProgressBarBase : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Progress")
	void SetPercent(float InPercent);

	UFUNCTION(BlueprintPure, Category = "Prism UI | Progress")
	float GetPercent() const { return CurrentPercent; }

protected:
	UPROPERTY(EditAnywhere, Category = "Prism UI | Progress", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CurrentPercent = 0.0f;
};
