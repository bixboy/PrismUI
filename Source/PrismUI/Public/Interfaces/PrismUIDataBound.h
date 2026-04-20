#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PrismUIDataBound.generated.h"


UINTERFACE(MinimalAPI, BlueprintType)
class UPrismUIDataBound : public UInterface
{
	GENERATED_BODY()
};


/**
 * IPrismUIDataBound — Core interface for data-driven widgets.
 * Allows passing arbitrary data objects into a widget for visual binding.
 */
class PRISMUI_API IPrismUIDataBound
{
	GENERATED_BODY()

public:
	/** 
	 * Binds a data object to this widget. 
	 * The implementation should cast the object to the expected type and refresh visuals.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prism UI | Data")
	void BindData(UObject* Payload);
};
