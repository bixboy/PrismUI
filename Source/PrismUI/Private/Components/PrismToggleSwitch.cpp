#include "Components/PrismToggleSwitch.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Border.h"
#include "Components/SizeBox.h"
#include "TimerManager.h"


UPrismToggleSwitch::UPrismToggleSwitch(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UPrismToggleSwitch::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
		return;

    if (RootSizer)
    {
        Tree->RootWidget = RootSizer;
        return;
    }

    RootSizer = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    RootSizer->SetWidthOverride(44.0f);
    RootSizer->SetHeightOverride(22.0f);
    RootSizer->SetVisibility(ESlateVisibility::Visible); 

	if (!Tree->RootWidget)
		Tree->RootWidget = RootSizer;

	RootOverlay = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
    RootOverlay->SetVisibility(ESlateVisibility::Visible);
    RootSizer->AddChild(RootOverlay);

	// 1. Track
	TrackBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
    TrackBorder->SetVisibility(ESlateVisibility::Visible);

	if (UOverlaySlot* TrackSlot = RootOverlay->AddChildToOverlay(TrackBorder))
	{
		TrackSlot->SetHorizontalAlignment(HAlign_Fill);
		TrackSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// 2. Thumb
    USizeBox* ThumbSizer = Tree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
    ThumbSizer->SetWidthOverride(18.0f);
    ThumbSizer->SetHeightOverride(18.0f);
    ThumbSizer->SetVisibility(ESlateVisibility::Visible);

	ThumbBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
    ThumbBorder->SetVisibility(ESlateVisibility::Visible);
    ThumbSizer->AddChild(ThumbBorder);

	if (UOverlaySlot* ThumbSlot = RootOverlay->AddChildToOverlay(ThumbSizer))
	{
		ThumbSlot->SetHorizontalAlignment(HAlign_Left);
		ThumbSlot->SetVerticalAlignment(VAlign_Center);
        ThumbSlot->SetPadding(FMargin(2.f));
	}
}

void UPrismToggleSwitch::SetIsChecked(bool bInIsChecked, bool bInBroadcast)
{
	Super::SetIsChecked(bInIsChecked, bInBroadcast);
}

bool UPrismToggleSwitch::TickTransitions(float DeltaTime)
{
    // 1. Process base toggle animation (ToggleAlpha)
    bool bBaseTicking = Super::TickTransitions(DeltaTime);

    // 2. Process Interactive State (Hover/Press)
    const EPrismWidgetState State = GetCurrentState();
    TargetInteractiveAlpha = (State == EPrismWidgetState::Normal || State == EPrismWidgetState::Disabled) ? 0.0f : 1.0f;
    
    InteractiveAlpha = FMath::FInterpTo(InteractiveAlpha, TargetInteractiveAlpha, DeltaTime, AnimationSpeed);

    bool bInteractiveTicking = !FMath::IsNearlyEqual(InteractiveAlpha, TargetInteractiveAlpha, 0.001f);

    // 3. Update Visuals
    UpdateThumbPosition(ToggleAlpha);
    OnVisualsUpdated(CurrentVisualAlpha);

    return bBaseTicking || bInteractiveTicking;
}

void UPrismToggleSwitch::UpdateThumbPosition(float InAlpha)
{
	if (!ThumbBorder)
		return;

	if (UOverlaySlot* ThumbSlot = RootOverlay ? Cast<UOverlaySlot>(ThumbBorder->GetParent()->Slot) : nullptr)
	{
        FMargin NewPadding;
        const float TotalSlideWidth = 22.0f;
        
        NewPadding.Left = 2.0f + (InAlpha * TotalSlideWidth);
        NewPadding.Top = 2.0f;
        NewPadding.Bottom = 2.0f;
        
        ThumbSlot->SetPadding(NewPadding);
	}
}

void UPrismToggleSwitch::OnVisualsUpdated(float InBlendValue)
{
	// Extension point for child classes like JupiterToggleSwitch
}

void UPrismToggleSwitch::OnStyleApplied(const FPrismUIWidgetStyle& InStyleData)
{
	Super::OnStyleApplied(InStyleData);

	if (!TrackBorder || !ThumbBorder)
		return;

	FSlateBrush TrackBrush;
	TrackBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
    
    const float PillRounding = 16.f; 
	TrackBrush.OutlineSettings.CornerRadii = FVector4(PillRounding, PillRounding, PillRounding, PillRounding);
	
    TrackBrush.OutlineSettings.Width = 1.0f;
    TrackBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
    TrackBrush.OutlineSettings.Color = FLinearColor(0.5f, 0.55f, 0.6f, 0.3f);

    // Background color blending between Off and On states
	FLinearColor OffColor = FLinearColor(0.12f, 0.12f, 0.15f, 0.8f);
	FLinearColor OnColor = FLinearColor(0.0f, 0.45f, 0.6f, 1.0f);
	
	TrackBrush.TintColor = FSlateColor(FMath::Lerp(OffColor, OnColor, ToggleAlpha));
	TrackBorder->SetBrush(TrackBrush);

	// Thumb Style
	FSlateBrush ThumbBrush;
	ThumbBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
	ThumbBrush.OutlineSettings.CornerRadii = FVector4(16.f, 16.f, 16.f, 16.f);
    ThumbBrush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
    
	ThumbBrush.TintColor = FLinearColor::White;
	ThumbBorder->SetBrush(ThumbBrush);
}
