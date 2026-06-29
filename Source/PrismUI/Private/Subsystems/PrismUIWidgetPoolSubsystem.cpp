#include "Subsystems/PrismUIWidgetPoolSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/PrismPoolableWidget.h"


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
			
			if (RecycledWidget->Implements<UPrismPoolableWidget>())
			{
				IPrismPoolableWidget::Execute_OnAcquiredFromPool(RecycledWidget);
			}

			UE_LOG(LogTemp, Display, TEXT("PrismUI: Reusing widget from pool: %s"), *InWidgetClass->GetName());
			return RecycledWidget;
		}
	}

	APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	UUserWidget* NewWidget = CreateWidget<UUserWidget>(PC, InWidgetClass);
	
	if (NewWidget && NewWidget->Implements<UPrismPoolableWidget>())
	{
		IPrismPoolableWidget::Execute_OnAcquiredFromPool(NewWidget);
	}

	UE_LOG(LogTemp, Display, TEXT("PrismUI: Pool empty. Creating new widget: %s"), *InWidgetClass->GetName());
	
	return NewWidget;
}

void UPrismUIWidgetPoolSubsystem::ReleaseWidget(UUserWidget* InWidget)
{
	if (!InWidget)
		return;

	if (InWidget->Implements<UPrismPoolableWidget>())
	{
		IPrismPoolableWidget::Execute_OnReleasedToPool(InWidget);
	}

	InWidget->SetVisibility(ESlateVisibility::Collapsed);

	const TSubclassOf<UUserWidget> WidgetClass = InWidget->GetClass();
	Pools.FindOrAdd(WidgetClass).InactiveWidgets.AddUnique(InWidget);
	
	UE_LOG(LogTemp, Display, TEXT("PrismUI: Released widget to pool: %s"), *WidgetClass->GetName());
}

void UPrismUIWidgetPoolSubsystem::ClearAllPools()
{
	Pools.Empty();
}
