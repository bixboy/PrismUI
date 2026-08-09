#include "Components/PrismRadialMenu.h"
#include "Utilities/PrismUI_Builder.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/BackgroundBlur.h"

UPrismRadialMenu::UPrismRadialMenu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bHasScriptImplementedTick = false;
}

void UPrismRadialMenu::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree) return;

	PRISM_CREATE_ROOT(UCanvasPanel, RootCanvas);
	MainCanvas = RootCanvas;

	PRISM_CREATE(UImage, BlurImg);
	BlurImg->SetColorAndOpacity(FLinearColor(0,0,0,0.0f)); // Invisible for now to avoid the grey square
	BlurImg->SetVisibility(ESlateVisibility::Collapsed);
	BackgroundBlur = BlurImg;

	if (UCanvasPanelSlot* BlurSlot = MainCanvas->AddChildToCanvas(BackgroundBlur))
	{
		BlurSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		BlurSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		BlurSlot->SetSize(FVector2D(Radius * 2.5f, Radius * 2.5f));
	}

	PRISM_CREATE(UImage, HighlightImg);
	HighlightImg->SetColorAndOpacity(FLinearColor::White);
	HighlightImg->SetVisibility(ESlateVisibility::Hidden); // Hidden by default
	CenterHighlight = HighlightImg;

	if (UCanvasPanelSlot* HighSlot = MainCanvas->AddChildToCanvas(CenterHighlight))
	{
		HighSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		HighSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		HighSlot->SetSize(FVector2D(Radius * 2.2f, Radius * 2.2f));
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UPrismRadialMenu::AddSegment(FName InID, FText InLabel, UTexture2D* InIcon, FLinearColor InColor)
{
	FPrismRadialSegment NewSegment;
	NewSegment.ID = InID;
	NewSegment.Label = InLabel;
	NewSegment.Icon = InIcon;
	NewSegment.Color = InColor;

	if (!MainCanvas || !WidgetTree) return;

	UWidgetTree* Tree = WidgetTree;

	// Visuals for Segment
	PRISM_CREATE(UImage, IconImg);
	if (InIcon)
	{
		IconImg->SetBrushFromTexture(InIcon);
	}
	IconImg->SetColorAndOpacity(InColor);
	IconImg->SetVisibility(ESlateVisibility::HitTestInvisible);

	PRISM_CREATE(UTextBlock, LabelTxt);
	LabelTxt->SetText(InLabel);
	LabelTxt->SetColorAndOpacity(FSlateColor(InColor));
	LabelTxt->SetJustification(ETextJustify::Center);
	LabelTxt->SetVisibility(ESlateVisibility::HitTestInvisible);

	// Add to Canvas
	if (UCanvasPanelSlot* IconSlot = MainCanvas->AddChildToCanvas(IconImg))
	{
		IconSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		IconSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		IconSlot->SetSize(FVector2D(48.f, 48.f));
	}

	if (UCanvasPanelSlot* LabelSlot = MainCanvas->AddChildToCanvas(LabelTxt))
	{
		LabelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		LabelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		LabelSlot->SetSize(FVector2D(120.f, 30.f));
	}

	NewSegment.IconWidget = IconImg;
	NewSegment.LabelWidget = LabelTxt;

	Segments.Add(NewSegment);

	// Re-layout all segments
	float AngleStep = 360.0f / FMath::Max(1, Segments.Num());
	for (int32 i = 0; i < Segments.Num(); ++i)
	{
		float AngleDeg = AngleStep * i - 90.0f; // Start at top
		float AngleRad = FMath::DegreesToRadians(AngleDeg);

		FVector2D Offset = FVector2D(FMath::Cos(AngleRad), FMath::Sin(AngleRad)) * Radius;

		if (UCanvasPanelSlot* IconSlot = Cast<UCanvasPanelSlot>(Segments[i].IconWidget->Slot))
		{
			IconSlot->SetPosition(Offset);
		}
		
		if (UCanvasPanelSlot* LabelSlot = Cast<UCanvasPanelSlot>(Segments[i].LabelWidget->Slot))
		{
			FVector2D LabelOffset = Offset + FVector2D(0.f, 40.f); // Below icon
			LabelSlot->SetPosition(LabelOffset);
		}
	}
}

void UPrismRadialMenu::ClearSegments()
{
	for (const FPrismRadialSegment& Seg : Segments)
	{
		if (Seg.IconWidget) Seg.IconWidget->RemoveFromParent();
		if (Seg.LabelWidget) Seg.LabelWidget->RemoveFromParent();
	}
	Segments.Empty();
}

void UPrismRadialMenu::OpenMenu()
{
	SetVisibility(ESlateVisibility::Visible);
	HoveredSegmentIndex = -1;
	
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			MouseStartLocation = FVector2D(MouseX, MouseY);
		}
	}
}

FName UPrismRadialMenu::CloseMenuAndGetSelection()
{
	SetVisibility(ESlateVisibility::Hidden);
	
	if (HoveredSegmentIndex >= 0 && HoveredSegmentIndex < Segments.Num())
	{
		FName SelectedID = Segments[HoveredSegmentIndex].ID;
		OnSegmentSelected.Broadcast(SelectedID);
		return SelectedID;
	}
	
	return NAME_None;
}

void UPrismRadialMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GetVisibility() != ESlateVisibility::Visible || Segments.Num() == 0)
		return;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			FVector2D CurrentMouse(MouseX, MouseY);
			FVector2D Delta = CurrentMouse - MouseStartLocation;

			if (Delta.Length() > InnerDeadZone)
			{
				float AngleRad = FMath::Atan2(Delta.Y, Delta.X);
				float AngleDeg = FMath::RadiansToDegrees(AngleRad);
				
				// Normalize to 0-360
				if (AngleDeg < 0) AngleDeg += 360.0f;

				// Offset by 90 degrees since first item is at top (-90) and half a step
				float AngleStep = 360.0f / Segments.Num();
				float ShiftedAngle = AngleDeg + 90.0f + (AngleStep * 0.5f);
				
				if (ShiftedAngle >= 360.0f) ShiftedAngle -= 360.0f;

				int32 NewHoverIndex = FMath::FloorToInt(ShiftedAngle / AngleStep) % Segments.Num();
				UpdateHoverState(NewHoverIndex);
			}
			else
			{
				UpdateHoverState(-1);
			}
		}
	}
}

void UPrismRadialMenu::UpdateHoverState(int32 NewHoverIndex)
{
	if (HoveredSegmentIndex == NewHoverIndex)
		return;

	// Reset old
	if (HoveredSegmentIndex >= 0 && HoveredSegmentIndex < Segments.Num())
	{
		Segments[HoveredSegmentIndex].IconWidget->SetRenderScale(FVector2D(1.0f, 1.0f));
		Segments[HoveredSegmentIndex].LabelWidget->SetRenderScale(FVector2D(1.0f, 1.0f));
	}

	HoveredSegmentIndex = NewHoverIndex;

	// Highlight new
	if (HoveredSegmentIndex >= 0 && HoveredSegmentIndex < Segments.Num())
	{
		Segments[HoveredSegmentIndex].IconWidget->SetRenderScale(FVector2D(1.3f, 1.3f));
		Segments[HoveredSegmentIndex].LabelWidget->SetRenderScale(FVector2D(1.1f, 1.1f));
		
		// Move highlight graphic
		if (CenterHighlight)
		{
			CenterHighlight->SetVisibility(ESlateVisibility::HitTestInvisible);
			
			float AngleStep = 360.0f / Segments.Num();
			float AngleDeg = AngleStep * HoveredSegmentIndex - 90.0f;
			CenterHighlight->SetRenderTransformAngle(AngleDeg);
			CenterHighlight->SetColorAndOpacity(Segments[HoveredSegmentIndex].Color.CopyWithNewOpacity(0.3f));
		}
	}
	else
	{
		if (CenterHighlight)
			CenterHighlight->SetVisibility(ESlateVisibility::Hidden);
	}
}
