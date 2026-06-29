#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PrismUISelectable.generated.h"


UINTERFACE(MinimalAPI, BlueprintType)
class UPrismUISelectable : public UInterface
{
	GENERATED_BODY()
};


/**
 * IPrismUISelectable — Interface for widgets that support selection states.
 * Useful for radio buttons, tabs, and list items.
 */
class PRISMUI_API IPrismUISelectable
{
	GENERATED_BODY()

public:
	/** Sets the selection state of the widget. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prism UI | Selection")
	void SetIsSelected(bool bInIsSelected);

	/** Returns true if the widget is currently selected. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prism UI | Selection")
	bool IsSelected() const;

	/** Called when the widget is clicked while already selected. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Prism UI | Selection")
	void OnSelectionConfirmed();
};
