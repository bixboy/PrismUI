#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PrismPoolableWidget.generated.h"


UINTERFACE(MinimalAPI, BlueprintType)
class UPrismPoolableWidget : public UInterface
{
	GENERATED_BODY()
};


class PRISMUI_API IPrismPoolableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prism UI | Pooling")
	void OnAcquiredFromPool();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prism UI | Pooling")
	void OnReleasedToPool();
};
