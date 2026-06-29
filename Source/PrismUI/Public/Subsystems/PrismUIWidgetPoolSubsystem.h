#pragma once
#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "PrismUIWidgetPoolSubsystem.generated.h"

class UUserWidget;


USTRUCT()
struct FPrismWidgetPool
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UUserWidget>> InactiveWidgets;
};


UCLASS()
class PRISMUI_API UPrismUIWidgetPoolSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	// --- Lifecycle ---

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Pooling Logic ---

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Pooling", meta = (DeterminesOutputType = "InWidgetClass"))
	UUserWidget* AcquireWidget(TSubclassOf<UUserWidget> InWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Pooling")
	void ReleaseWidget(UUserWidget* InWidget);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Pooling")
	void ClearAllPools();

private:
	UPROPERTY()
	TMap<TSubclassOf<UUserWidget>, FPrismWidgetPool> Pools;
};
