// Copyright Prism UI. All Rights Reserved.

#include "Subsystems/PrismUIWidgetPoolSubsystem.h"
#include "Blueprint/UserWidget.h"

void UPrismUIWidgetPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPrismUIWidgetPoolSubsystem::Deinitialize()
{
	ClearAllPools();
	Super::Deinitialize();
}

UUserWidget* UPrismUIWidgetPoolSubsystem::AcquireWidget(TSubclassOf<UUserWidget> InWidgetClass)
{
	if (!InWidgetClass)
		return nullptr;

	FPrismWidgetPool& Pool = Pools.FindOrAdd(InWidgetClass);
	
	if (Pool.InactiveWidgets.Num() > 0)
	{
		TObjectPtr<UUserWidget> RecycledWidget = Pool.InactiveWidgets.Pop();
		if (IsValid(RecycledWidget))
		{
			RecycledWidget->SetVisibility(ESlateVisibility::Visible);
			UE_LOG(LogTemp, Display, TEXT("PrismUI: Reusing widget from pool: %s"), *InWidgetClass->GetName());
			return RecycledWidget;
		}
	}

	// Pool empty or contains invalid entries, create brand new
	UUserWidget* NewWidget = CreateWidget<UUserWidget>(GetGameInstance(), InWidgetClass);
	
	UE_LOG(LogTemp, Display, TEXT("PrismUI: Pool empty. Creating new widget: %s"), *InWidgetClass->GetName());
	
	return NewWidget;
}

void UPrismUIWidgetPoolSubsystem::ReleaseWidget(UUserWidget* InWidget)
{
	if (!InWidget)
		return;

	// CRITICAL: We DO NOT call RemoveFromParent() to preserve Slot constraints.
	InWidget->SetVisibility(ESlateVisibility::Collapsed);

	const TSubclassOf<UUserWidget> WidgetClass = InWidget->GetClass();
	Pools.FindOrAdd(WidgetClass).InactiveWidgets.AddUnique(InWidget);
	
	UE_LOG(LogTemp, Display, TEXT("PrismUI: Released widget to pool: %s"), *WidgetClass->GetName());
}

void UPrismUIWidgetPoolSubsystem::ClearAllPools()
{
	Pools.Empty();
}
