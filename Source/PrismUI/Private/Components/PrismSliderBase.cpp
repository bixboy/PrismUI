#include "Components/PrismSliderBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Slider.h"
#include "Components/ProgressBar.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

void UPrismSliderBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (InternalSlider)
	{
		InternalSlider->OnValueChanged.AddUniqueDynamic(this, &UPrismSliderBase::HandleOnSliderValueChanged);
		InternalSlider->OnMouseCaptureEnd.AddUniqueDynamic(this, &UPrismSliderBase::SendCurrentValue);

		InternalSlider->SetMinValue(MinValue);
		InternalSlider->SetMaxValue(MaxValue);
		InternalSlider->SetValue(CurrentValue);
	}
}

void UPrismSliderBase::NativeDestruct()
{
	if (InternalSlider)
	{
		InternalSlider->OnValueChanged.RemoveAll(this);
		InternalSlider->OnMouseCaptureEnd.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPrismSliderBase::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree || RootOverlay)
		return;

	RootOverlay = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	
    if (!Tree->RootWidget)
	{
	    Tree->RootWidget = RootOverlay;
	}

	FillBar = Tree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	if (UOverlaySlot* FillSlot = RootOverlay->AddChildToOverlay(FillBar))
	{
		FillSlot->SetHorizontalAlignment(HAlign_Fill);
		FillSlot->SetVerticalAlignment(VAlign_Center);
	}

	InternalSlider = Tree->ConstructWidget<USlider>(USlider::StaticClass());
	if (UOverlaySlot* SliderSlot = RootOverlay->AddChildToOverlay(InternalSlider))
	{
		SliderSlot->SetHorizontalAlignment(HAlign_Fill);
		SliderSlot->SetVerticalAlignment(VAlign_Center);
	}
}

void UPrismSliderBase::OnVisualsUpdated(float InBlendValue)
{
    Super::OnVisualsUpdated(InBlendValue);
    
    if (InternalSlider)
    {
        const float NewScale = FMath::Lerp(1.0f, 1.25f, InBlendValue);
        InternalSlider->SetRenderScale(FVector2D(NewScale, NewScale));
    }
}

void UPrismSliderBase::SetValueRange(float InMin, float InMax)
{
	MinValue = InMin;
	MaxValue = InMax;

	if (InternalSlider)
	{
		InternalSlider->SetMinValue(InMin);
		InternalSlider->SetMaxValue(InMax);
	}

	SetValue(CurrentValue, false);
}

void UPrismSliderBase::SetValue(float InValue, bool bInBroadcast)
{
	float NewValue = FMath::Clamp(InValue, MinValue, MaxValue);

	if (StepSize > 0.0f)
	{
		const float SnapValue = FMath::RoundToFloat(NewValue / StepSize) * StepSize;
		NewValue = FMath::Clamp(SnapValue, MinValue, MaxValue);
	}

	if (FMath::IsNearlyEqual(CurrentValue, NewValue))
		return;

	CurrentValue = NewValue;
	
	if (InternalSlider && !InternalSlider->HasMouseCapture())
		InternalSlider->SetValue(CurrentValue);

	if (FillBar)
		FillBar->SetPercent(GetNormalizedValue());

	RefreshStyle();

	if (bInBroadcast)
		OnValueChanged.Broadcast(this, CurrentValue);
}

void UPrismSliderBase::SendCurrentValue()
{
	OnValueSend.Broadcast(this, CurrentValue);
	UE_LOG(LogTemp, Display, TEXT("PrismUI: Slider Value Sent - %f"), CurrentValue);
}

void UPrismSliderBase::HandleOnSliderValueChanged(float InNewValue)
{
    SetValue(InNewValue, true);
}

float UPrismSliderBase::GetNormalizedValue() const
{
	if (FMath::IsNearlyEqual(MinValue, MaxValue))
		return 0.0f;

	return (CurrentValue - MinValue) / (MaxValue - MinValue);
}
