#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/PrismUIWidgetPoolSubsystem.h"
#include "Components/PrismButtonBase.h"
#include "PrismUIFactory.generated.h"

/**
 * UPrismUIFactory
 * Static utility library providing a Fluent API for Prism UI construction.
 */
UCLASS()
class PRISMUI_API UPrismUIFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Spawns or acquires a button of the specified class.
	 */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Factory", meta = (WorldContext = "WorldContextObject"))
	static UPrismButtonBase* CreateButton(UObject* WorldContextObject, TSubclassOf<UPrismButtonBase> InClass)
	{
		if (!WorldContextObject || !InClass)
			return nullptr;

		UWorld* World = WorldContextObject->GetWorld();
		if (!World)
			return nullptr;

		UPrismUIWidgetPoolSubsystem* Pool = World->GetGameInstance()->GetSubsystem<UPrismUIWidgetPoolSubsystem>();
		return Cast<UPrismButtonBase>(Pool->AcquireWidget(InClass));
	}
};
