#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PrismUIWidgetPoolSubsystem.generated.h"

class UUserWidget;


/** Container for a specific class pool. */
USTRUCT()
struct FPrismWidgetPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> InactiveWidgets;
};


/**
 * UPrismUIWidgetPoolSubsystem — High-performance widget recycler.
 * Reduces GC pressure and instantiation overhead by pooling and reusing 
 * deactivated widgets.
 */
UCLASS()
class PRISMUI_API UPrismUIWidgetPoolSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// --- Lifecycle ---

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Pooling Logic ---

	/** 
	 * Retrieves a widget of the specified class from the pool or creates a new one 
	 * if the pool is empty. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Pooling", meta = (DeterminesOutputType = "InWidgetClass"))
	UUserWidget* AcquireWidget(TSubclassOf<UUserWidget> InWidgetClass);

	/** 
	 * Returns a widget to the pool for future reuse. 
	 * The widget will be collapsed but NOT removed from its parent to preserve
	 * layout slot constraints (Padding, Alignment).
	 */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Pooling")
	void ReleaseWidget(UUserWidget* InWidget);

	/** Clears all cached widgets from the pool. */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Pooling")
	void ClearAllPools();

private:
	/** Map of Widget Classes to their respective available instances. */
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, FPrismWidgetPool> Pools;
};
