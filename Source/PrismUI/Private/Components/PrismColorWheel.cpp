#include "Components/PrismColorWheel.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Spacer.h"
#include "Kismet/KismetMathLibrary.h"
#include "Input/Reply.h"

UPrismColorWheel::UPrismColorWheel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPrismColorWheel::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Image_Wheel)
	{
		BuildDefaultLayout();
	}

	UpdateSelectorPosition();
}

void UPrismColorWheel::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree || RootContainer)
		return;

	RootContainer = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	if (!Tree->RootWidget)
	{
		Tree->RootWidget = RootContainer;
	}

	// 1. Maintain 1:1 Aspect Ratio
	AspectSpacer = Tree->ConstructWidget<USpacer>(USpacer::StaticClass());
	if (UOverlaySlot* SpacerSlot = RootContainer->AddChildToOverlay(AspectSpacer))
	{
		// Force square sizing via spacer if needed, but usually handled by layout
	}

	// 2. The Color Wheel Background
	Image_Wheel = Tree->ConstructWidget<UImage>(UImage::StaticClass());
	if (UOverlaySlot* WheelSlot = RootContainer->AddChildToOverlay(Image_Wheel))
	{
		WheelSlot->SetHorizontalAlignment(HAlign_Fill);
		WheelSlot->SetVerticalAlignment(VAlign_Fill);
	}

	// 3. The Selector Handle
	Image_Selector = Tree->ConstructWidget<UImage>(UImage::StaticClass());
	if (UOverlaySlot* SelectorSlot = RootContainer->AddChildToOverlay(Image_Selector))
	{
		SelectorSlot->SetHorizontalAlignment(HAlign_Center);
		SelectorSlot->SetVerticalAlignment(VAlign_Center);
		
		// Small circle for the selector
		Image_Selector->SetDesiredSizeOverride(FVector2D(16.f, 16.f));
	}
}

void UPrismColorWheel::SetSelectedColor(FLinearColor InColor, bool bBroadcast)
{
	SelectedColor = InColor;
	UpdateSelectorPosition();

	if (bBroadcast)
	{
		OnColorChanged.Broadcast(SelectedColor);
	}
}

FReply UPrismColorWheel::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		ProcessMouseInput(InGeometry, InMouseEvent);
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	return FReply::Unhandled();
}

FReply UPrismColorWheel::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bIsDragging)
	{
		bIsDragging = false;
		OnColorCommitted.Broadcast(SelectedColor);
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply UPrismColorWheel::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging)
	{
		ProcessMouseInput(InGeometry, InMouseEvent);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void UPrismColorWheel::OnVisualsUpdated(float InBlendValue)
{
	Super::OnVisualsUpdated(InBlendValue);
	
	if (Image_Selector)
	{
		const float Scale = FMath::Lerp(1.0f, 1.2f, InBlendValue);
		Image_Selector->SetRenderScale(FVector2D(Scale, Scale));
	}
}

void UPrismColorWheel::ProcessMouseInput(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	const FVector2D LocalPos = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const FVector2D Size = InGeometry.GetLocalSize();
	const FVector2D Center = Size * 0.5f;
	const float Radius = FMath::Min(Size.X, Size.Y) * 0.5f;

	if (Radius <= 0.0f) return;

	FVector2D Dir = LocalPos - Center;
	float Dist = Dir.Size();
	
	// Normalize direction
	Dir.Normalize();

	// Calculate Saturation (Distance from center)
	float Saturation = FMath::Clamp(Dist / Radius, 0.0f, 1.0f);

	// Calculate Hue (Angle)
	// atan2 returns -PI to PI. We want 0 to 360.
	float AngleRad = FMath::Atan2(Dir.Y, Dir.X);
	float Hue = FMath::RadiansToDegrees(AngleRad);
	if (Hue < 0.0f) Hue += 360.0f;

	// Update selected color (Keep Value at 1.0 for the wheel)
	SelectedColor = UKismetMathLibrary::HSVToRGB(Hue, Saturation, 1.0f);
	
	UpdateSelectorPosition();
	OnColorChanged.Broadcast(SelectedColor);
}

void UPrismColorWheel::UpdateSelectorPosition()
{
	if (!Image_Selector || !RootContainer) return;

	const FVector2D Size = GetTickSpaceGeometry().GetLocalSize();
	if (Size.IsNearlyZero()) return;

	const FVector2D Center = Size * 0.5f;
	const float Radius = FMath::Min(Size.X, Size.Y) * 0.5f;

	float Hue, Saturation, Value, Alpha;
	UKismetMathLibrary::RGBToHSV(SelectedColor, Hue, Saturation, Value, Alpha);

	float AngleRad = FMath::DegreesToRadians(Hue);
	FVector2D Offset(FMath::Cos(AngleRad), FMath::Sin(AngleRad));
	
	FVector2D FinalPos = Center + (Offset * Saturation * Radius);
	
	// Since it's in an overlay centered, we use RenderTransform Translation
	Image_Selector->SetRenderTranslation(FinalPos - Center);
}
