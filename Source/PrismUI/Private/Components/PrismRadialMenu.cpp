// Copyright (c) Bixboy, 2026. All Rights Reserved.

#include "Components/PrismRadialMenu.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/BackgroundBlur.h"
#include "GameFramework/PlayerController.h"
#include "Rendering/DrawElements.h"
#include "Rendering/RenderingCommon.h"
#include "Styling/CoreStyle.h"
#include "Styling/AppStyle.h"
#include "Fonts/SlateFontInfo.h"
#include "Framework/Application/SlateApplication.h"

UPrismRadialMenu::UPrismRadialMenu(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bHasScriptImplementedTick = false;
	Radius = 220.0f;
	InnerRadius = 100.0f;
	InnerDeadZone = 40.0f;
	SubdivisionCount = 0;
	SeparatorGapAngle = 3.5f;
	StartAngleOffset = -90.0f;
	HoverExpansionDistance = 8.0f;
	IconDimensions = FVector2D(64.0f, 44.0f);
	bShowLabelsInSectors = true;
	bShowCenterReticle = true;
	bShowDirectionPointer = true;

	DefaultSectorFillColor = FLinearColor(0.015f, 0.025f, 0.038f, 0.78f);
	DefaultSectorOutlineColor = FLinearColor(0.0f, 0.75f, 1.0f, 0.28f);
	ActiveSectorFillColor = FLinearColor(0.0f, 0.75f, 0.95f, 0.85f);
	ActiveSectorOutlineColor = FLinearColor(0.4f, 0.95f, 1.0f, 1.0f);
	EmptySectorFillColor = FLinearColor(0.008f, 0.012f, 0.02f, 0.40f);
	ReticleColor = FLinearColor(0.0f, 0.75f, 1.0f, 0.45f);
	PointerColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.95f);
}

void UPrismRadialMenu::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
		return;

	// 1. Root Canvas
	UCanvasPanel* RootCanvas = Tree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	Tree->RootWidget = RootCanvas;
	MainCanvas = RootCanvas;

	// 2. Optional Soft Background Blur
	UImage* BlurImg = Tree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("BackgroundBlur"));
	BlurImg->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	BlurImg->SetVisibility(ESlateVisibility::Collapsed);
	BackgroundBlur = BlurImg;
	if (UCanvasPanelSlot* BlurSlot = MainCanvas->AddChildToCanvas(BackgroundBlur))
	{
		BlurSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		BlurSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		BlurSlot->SetSize(FVector2D(GetOuterRadius() * 2.5f, GetOuterRadius() * 2.5f));
	}

	// 3. Center Info Display (Overlay for Title, Subtitle, and Reticle text)
	CenterOverlay = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("CenterOverlay"));
	CenterOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UCanvasPanelSlot* CenterSlot = MainCanvas->AddChildToCanvas(CenterOverlay))
	{
		CenterSlot->SetAnchors(FAnchors(0.5f, 0.5f));
		CenterSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		CenterSlot->SetSize(FVector2D(InnerRadius * 1.6f, InnerRadius * 1.6f));
	}

	UVerticalBox* CenterVBox = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CenterVBox"));
	CenterOverlay->AddChildToOverlay(CenterVBox);
	if (UOverlaySlot* VBoxSlot = Cast<UOverlaySlot>(CenterVBox->Slot))
	{
		VBoxSlot->SetHorizontalAlignment(HAlign_Center);
		VBoxSlot->SetVerticalAlignment(VAlign_Center);
	}

	CenterTitleText = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CenterTitleText"));
	CenterTitleText->SetJustification(ETextJustify::Center);
	CenterTitleText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	FSlateFontInfo TitleFont = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 12.0f);
	CenterTitleText->SetFont(TitleFont);
	CenterVBox->AddChildToVerticalBox(CenterTitleText);

	CenterSubtitleText = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CenterSubtitleText"));
	CenterSubtitleText->SetJustification(ETextJustify::Center);
	CenterSubtitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 0.85f, 1.0f, 0.85f)));
	FSlateFontInfo SubFont = FCoreStyle::GetDefaultFontStyle(TEXT("Regular"), 8.5f);
	CenterSubtitleText->SetFont(SubFont);
	if (UVerticalBoxSlot* SubSlot = CenterVBox->AddChildToVerticalBox(CenterSubtitleText))
	{
		SubSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));
	}

	SetVisibility(ESlateVisibility::Hidden);
}

void UPrismRadialMenu::AddSegment(FName InID, FText InLabel, UTexture2D* InIcon, FLinearColor InColor)
{
	AddSegmentEx(InID, InLabel, FText::GetEmpty(), InIcon, InColor);
}

void UPrismRadialMenu::AddSegmentEx(FName InID, FText InLabel, FText InSubtitle, UTexture2D* InIcon, FLinearColor InColor)
{
	FPrismRadialSegment NewSegment;
	NewSegment.ID = InID;
	NewSegment.Label = InLabel;
	NewSegment.Subtitle = InSubtitle;
	NewSegment.Icon = InIcon;
	NewSegment.Color = InColor;
	NewSegment.bIsEnabled = true;
	NewSegment.HoverProgress = 0.0f;

	Segments.Add(NewSegment);
	RebuildSectorWidgets();
}

void UPrismRadialMenu::ClearSegments()
{
	for (FPrismRadialSegment& Seg : Segments)
	{
		if (Seg.IconWidget)
			Seg.IconWidget->RemoveFromParent();
		if (Seg.LabelWidget)
			Seg.LabelWidget->RemoveFromParent();
	}
	Segments.Empty();
	HoveredSegmentIndex = -1;

	if (CenterTitleText)
		CenterTitleText->SetText(FText::GetEmpty());
	if (CenterSubtitleText)
		CenterSubtitleText->SetText(FText::GetEmpty());
}

void UPrismRadialMenu::SetSegmentEnabled(int32 InIndex, bool bInEnabled)
{
	if (Segments.IsValidIndex(InIndex))
	{
		Segments[InIndex].bIsEnabled = bInEnabled;
		if (Segments[InIndex].IconWidget)
		{
			Segments[InIndex].IconWidget->SetColorAndOpacity(
				bInEnabled ? FLinearColor::White : FLinearColor(0.3f, 0.3f, 0.3f, 0.35f)
			);
		}
	}
}

void UPrismRadialMenu::SetSegmentData(int32 InIndex, FName InID, FText InLabel, FText InSubtitle, UTexture2D* InIcon, FLinearColor InColor)
{
	if (!Segments.IsValidIndex(InIndex))
		return;

	Segments[InIndex].ID = InID;
	Segments[InIndex].Label = InLabel;
	Segments[InIndex].Subtitle = InSubtitle;
	Segments[InIndex].Icon = InIcon;
	Segments[InIndex].Color = InColor;

	if (Segments[InIndex].IconWidget && InIcon)
	{
		Segments[InIndex].IconWidget->SetBrushFromTexture(InIcon);
		Segments[InIndex].IconWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	if (Segments[InIndex].LabelWidget)
		Segments[InIndex].LabelWidget->SetText(InLabel);
}

void UPrismRadialMenu::SetSubdivisionCount(int32 InCount)
{
	SubdivisionCount = FMath::Clamp(InCount, 0, 32);
	RebuildSectorWidgets();
}

int32 UPrismRadialMenu::GetEffectiveSubdivisionCount() const
{
	if (SubdivisionCount > 0)
		return SubdivisionCount;

	return FMath::Max(1, Segments.Num());
}

void UPrismRadialMenu::SetWheelRadii(float InInnerRadius, float InOuterRadius)
{
	InnerRadius = FMath::Max(20.0f, InInnerRadius);
	Radius = FMath::Max(InnerRadius + 30.0f, InOuterRadius);
	RebuildSectorWidgets();
}

void UPrismRadialMenu::SetSeparatorGapAngle(float InGapDegrees)
{
	SeparatorGapAngle = FMath::Clamp(InGapDegrees, 0.0f, 20.0f);
}

void UPrismRadialMenu::SetStartAngleOffset(float InAngleDegrees)
{
	StartAngleOffset = InAngleDegrees;
	RebuildSectorWidgets();
}

void UPrismRadialMenu::SetHoverExpansionDistance(float InDistance)
{
	HoverExpansionDistance = FMath::Clamp(InDistance, 0.0f, 30.0f);
}

void UPrismRadialMenu::SetCenterInfo(const FText& InTitle, const FText& InSubtitle)
{
	if (CenterTitleText)
		CenterTitleText->SetText(InTitle);
	if (CenterSubtitleText)
		CenterSubtitleText->SetText(InSubtitle);
}

void UPrismRadialMenu::OpenMenu()
{
	SetVisibility(ESlateVisibility::Visible);
	HoveredSegmentIndex = -1;
	OpenAnimProgress = 0.0f;
	bIsOpening = true;
	SetRenderScale(FVector2D(0.88f, 0.88f));
	SetRenderOpacity(0.0f);

	for (FPrismRadialSegment& Seg : Segments)
		Seg.HoverProgress = 0.0f;

	if (CenterTitleText)
		CenterTitleText->SetText(FText::GetEmpty());
	if (CenterSubtitleText)
		CenterSubtitleText->SetText(FText::GetEmpty());

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
			MouseStartLocation = FVector2D(MouseX, MouseY);
	}
}

FName UPrismRadialMenu::CloseMenuAndGetSelection()
{
	SetVisibility(ESlateVisibility::Hidden);

	if (HoveredSegmentIndex >= 0 && HoveredSegmentIndex < Segments.Num())
	{
		if (Segments[HoveredSegmentIndex].bIsEnabled)
		{
			FName SelectedID = Segments[HoveredSegmentIndex].ID;
			OnSegmentSelected.Broadcast(SelectedID);
			return SelectedID;
		}
	}

	return NAME_None;
}

void UPrismRadialMenu::SelectSegmentByIndex(int32 InIndex)
{
	if (Segments.IsValidIndex(InIndex) && Segments[InIndex].bIsEnabled)
	{
		UpdateHoverState(InIndex);
		OnSegmentSelected.Broadcast(Segments[InIndex].ID);
	}
}

void UPrismRadialMenu::RebuildSectorWidgets()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree || !MainCanvas)
		return;

	const int32 TotalSectors = GetEffectiveSubdivisionCount();
	if (TotalSectors <= 0)
		return;

	const float AngleStep = 360.0f / TotalSectors;
	const float ContentRadius = (InnerRadius + GetOuterRadius()) * 0.5f;

	for (int32 i = 0; i < Segments.Num(); ++i)
	{
		FPrismRadialSegment& Seg = Segments[i];

		// Ensure IconWidget exists
		if (!Seg.IconWidget)
		{
			Seg.IconWidget = Tree->ConstructWidget<UImage>(UImage::StaticClass());
			Seg.IconWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			MainCanvas->AddChildToCanvas(Seg.IconWidget);
		}

		// Ensure LabelWidget exists
		if (!Seg.LabelWidget)
		{
			Seg.LabelWidget = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
			Seg.LabelWidget->SetJustification(ETextJustify::Center);
			Seg.LabelWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			FSlateFontInfo Font = FCoreStyle::GetDefaultFontStyle(TEXT("Bold"), 11.5f);
			Seg.LabelWidget->SetFont(Font);
			Seg.LabelWidget->SetShadowOffset(FVector2D(1.0f, 1.2f));
			Seg.LabelWidget->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f));
			MainCanvas->AddChildToCanvas(Seg.LabelWidget);
		}

		if (Seg.Icon)
		{
			Seg.IconWidget->SetBrushFromTexture(Seg.Icon);
			Seg.IconWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			Seg.IconWidget->SetVisibility(ESlateVisibility::Collapsed);
		}

		Seg.LabelWidget->SetText(Seg.Label);

		// Calculate sector midpoint position
		const float AngleDeg = StartAngleOffset + i * AngleStep;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);
		const FVector2D ContentOffset = FVector2D(FMath::Cos(AngleRad), FMath::Sin(AngleRad)) * ContentRadius;

		if (UCanvasPanelSlot* IconSlot = Cast<UCanvasPanelSlot>(Seg.IconWidget->Slot))
		{
			IconSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			IconSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			IconSlot->SetSize(IconDimensions);
			IconSlot->SetPosition(ContentOffset - FVector2D(0.0f, 10.0f));
		}

		if (UCanvasPanelSlot* LabelSlot = Cast<UCanvasPanelSlot>(Seg.LabelWidget->Slot))
		{
			LabelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
			LabelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			LabelSlot->SetSize(FVector2D(130.0f, 26.0f));

			if (Seg.Icon && bShowLabelsInSectors)
			{
				// Position label cleanly below icon
				LabelSlot->SetPosition(ContentOffset + FVector2D(0.0f, IconDimensions.Y * 0.5f + 4.0f));
				Seg.LabelWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			else if (!Seg.Icon)
			{
				// Centered text for text-only sectors
				LabelSlot->SetPosition(ContentOffset);
				Seg.LabelWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
			else
			{
				Seg.LabelWidget->SetVisibility(ESlateVisibility::Collapsed);
			}
		}
	}
}

void UPrismRadialMenu::UpdateHoverState(int32 NewHoverIndex)
{
	if (HoveredSegmentIndex == NewHoverIndex)
		return;

	HoveredSegmentIndex = NewHoverIndex;

	if (HoveredSegmentIndex >= 0 && HoveredSegmentIndex < Segments.Num())
	{
		const FPrismRadialSegment& ActiveSeg = Segments[HoveredSegmentIndex];
		if (CenterTitleText)
			CenterTitleText->SetText(ActiveSeg.Label);

		if (CenterSubtitleText)
		{
			if (!ActiveSeg.Subtitle.IsEmpty())
				CenterSubtitleText->SetText(ActiveSeg.Subtitle);
			else
				CenterSubtitleText->SetText(NSLOCTEXT("PrismUI", "EquippedActive", "ACTIF // ÉQUIPÉ"));
		}

		OnSegmentHovered.Broadcast(ActiveSeg.ID, HoveredSegmentIndex);
	}
	else if (HoveredSegmentIndex >= Segments.Num())
	{
		// Hovering an empty slot
		if (CenterTitleText)
			CenterTitleText->SetText(NSLOCTEXT("PrismUI", "EmptySlot", "[VIDE]"));
		if (CenterSubtitleText)
			CenterSubtitleText->SetText(NSLOCTEXT("PrismUI", "SlotAvailable", "EMPLACEMENT DISPONIBLE"));
	}
	else
	{
		if (CenterTitleText)
			CenterTitleText->SetText(FText::GetEmpty());
		if (CenterSubtitleText)
			CenterSubtitleText->SetText(FText::GetEmpty());

		OnSegmentHovered.Broadcast(NAME_None, -1);
	}
}

void UPrismRadialMenu::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (GetVisibility() != ESlateVisibility::Visible)
		return;

	// 1. Smooth Holographic Entrance Animation
	if (bIsOpening)
	{
		OpenAnimProgress = FMath::FInterpTo(OpenAnimProgress, 1.0f, InDeltaTime, 16.0f);
		SetRenderScale(FVector2D(FMath::Lerp(0.88f, 1.0f, OpenAnimProgress)));
		SetRenderOpacity(OpenAnimProgress);

		if (OpenAnimProgress >= 0.999f)
		{
			OpenAnimProgress = 1.0f;
			bIsOpening = false;
		}
	}

	// 2. Smooth Per-Segment Hover Interpolation
	for (int32 i = 0; i < Segments.Num(); ++i)
	{
		const float TargetHover = (i == HoveredSegmentIndex && Segments[i].bIsEnabled) ? 1.0f : 0.0f;
		Segments[i].HoverProgress = FMath::FInterpTo(Segments[i].HoverProgress, TargetHover, InDeltaTime, 14.0f);

		if (Segments[i].IconWidget)
		{
			const float Scale = FMath::Lerp(1.0f, 1.10f, Segments[i].HoverProgress);
			Segments[i].IconWidget->SetRenderScale(FVector2D(Scale, Scale));

			// Preserve 100% natural, full colors for textures (do NOT tint with cyan/blue)
			const float Opacity = Segments[i].bIsEnabled ? FMath::Lerp(0.92f, 1.0f, Segments[i].HoverProgress) : 0.35f;
			Segments[i].IconWidget->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, Opacity));
		}

		if (Segments[i].LabelWidget)
		{
			const FLinearColor TextCol = FMath::Lerp(
				Segments[i].bIsEnabled ? FLinearColor(0.90f, 0.94f, 0.97f, 0.95f) : FLinearColor(0.45f, 0.50f, 0.55f, 0.5f),
				FLinearColor::White,
				Segments[i].HoverProgress
			);
			Segments[i].LabelWidget->SetColorAndOpacity(FSlateColor(TextCol));
		}
	}

	// 3. Mouse / Direction Calculation
	const int32 TotalSectors = GetEffectiveSubdivisionCount();
	if (TotalSectors <= 0)
		return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
		return;

	const FVector2D LocalSize = MyGeometry.GetLocalSize();
	if (LocalSize.X <= 1.0f || LocalSize.Y <= 1.0f)
		return;

	const FVector2D Center = LocalSize * 0.5f;
	FVector2D Delta = FVector2D::ZeroVector;
	bool bHasInput = false;

	if (PC->bShowMouseCursor && FSlateApplication::IsInitialized())
	{
		const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
		const FVector2D LocalCursor = MyGeometry.AbsoluteToLocal(CursorPos);
		Delta = LocalCursor - Center;
		bHasInput = true;
	}
	else
	{
		float MouseX, MouseY;
		if (PC->GetMousePosition(MouseX, MouseY))
		{
			const FVector2D CurrentMouse(MouseX, MouseY);
			Delta = CurrentMouse - MouseStartLocation;
			bHasInput = true;
		}
	}

	if (bHasInput && Delta.Length() > InnerDeadZone)
	{
		const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Delta.Y, Delta.X));
		const float AngleStep = 360.0f / TotalSectors;

		// Normalize angle relative to StartAngleOffset so sector 0 spans [-AngleStep/2, +AngleStep/2] around StartAngleOffset
		const float RelAngle = FRotator::ClampAxis(AngleDeg - StartAngleOffset + (AngleStep * 0.5f));
		const int32 TargetIndex = FMath::Clamp(FMath::FloorToInt(RelAngle / AngleStep), 0, TotalSectors - 1);
		UpdateHoverState(TargetIndex);
	}
	else
	{
		UpdateHoverState(-1);
	}
}

int32 UPrismRadialMenu::NativePaint(
	const FPaintArgs& Args,
	const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FWidgetStyle& InWidgetStyle,
	bool bParentEnabled) const
{
	const int32 TotalSectors = GetEffectiveSubdivisionCount();
	if (TotalSectors <= 0)
		return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const FVector2D Center = AllottedGeometry.GetLocalSize() * 0.5f;
	const float AngleStep = 360.0f / TotalSectors;
	const float HalfSpan = (AngleStep - SeparatorGapAngle) * 0.5f;

	// --- 1. Procedural Annular Sector Backgrounds & Outlines ---
	for (int32 i = 0; i < TotalSectors; ++i)
	{
		const float CenterAngle = StartAngleOffset + i * AngleStep;
		const float StartDeg = CenterAngle - HalfSpan;
		const float EndDeg = CenterAngle + HalfSpan;

		if (i < Segments.Num())
		{
			const float Hover = Segments[i].HoverProgress;
			const FLinearColor Fill = FMath::Lerp(DefaultSectorFillColor, ActiveSectorFillColor, Hover);
			const FLinearColor Outline = FMath::Lerp(DefaultSectorOutlineColor, ActiveSectorOutlineColor, Hover);
			const float CurOuterR = GetOuterRadius() + HoverExpansionDistance * Hover;
			const float Thick = FMath::Lerp(1.2f, 2.4f, Hover);

			DrawAnnularSector(OutDrawElements, LayerId, AllottedGeometry, Center, InnerRadius, CurOuterR, StartDeg, EndDeg, Fill, Outline, Thick);
		}
		else
		{
			// Empty Tactical Slot
			const bool bEmptyHover = (i == HoveredSegmentIndex);
			const FLinearColor Fill = bEmptyHover ? EmptySectorFillColor.CopyWithNewOpacity(0.65f) : EmptySectorFillColor;
			const FLinearColor Outline = bEmptyHover ? DefaultSectorOutlineColor.CopyWithNewOpacity(0.55f) : DefaultSectorOutlineColor.CopyWithNewOpacity(0.18f);

			DrawAnnularSector(OutDrawElements, LayerId, AllottedGeometry, Center, InnerRadius, GetOuterRadius(), StartDeg, EndDeg, Fill, Outline, 1.0f);
		}
	}

	// --- 2. Inner Circle Border ---
	const int32 CircleSteps = 48;
	TArray<FVector2f> CirclePoints;
	CirclePoints.Reserve(CircleSteps + 1);
	for (int32 i = 0; i <= CircleSteps; ++i)
	{
		const float Rad = (static_cast<float>(i) / CircleSteps) * 2.0f * PI;
		CirclePoints.Add(FVector2f(Center.X + InnerRadius * FMath::Cos(Rad), Center.Y + InnerRadius * FMath::Sin(Rad)));
	}
	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(),
		CirclePoints,
		ESlateDrawEffect::None,
		ReticleColor.CopyWithNewOpacity(0.35f),
		true,
		1.0f
	);

	// --- 3. Center Reticle Crosshairs & Targeting Dot ---
	if (bShowCenterReticle)
	{
		const float TickLen = 7.0f;
		TArray<FVector2f> Ticks;
		// Top
		Ticks.Add(FVector2f(Center.X, Center.Y - InnerRadius));
		Ticks.Add(FVector2f(Center.X, Center.Y - InnerRadius + TickLen));
		// Bottom
		Ticks.Add(FVector2f(Center.X, Center.Y + InnerRadius));
		Ticks.Add(FVector2f(Center.X, Center.Y + InnerRadius - TickLen));
		// Left
		Ticks.Add(FVector2f(Center.X - InnerRadius, Center.Y));
		Ticks.Add(FVector2f(Center.X - InnerRadius + TickLen, Center.Y));
		// Right
		Ticks.Add(FVector2f(Center.X + InnerRadius, Center.Y));
		Ticks.Add(FVector2f(Center.X + InnerRadius - TickLen, Center.Y));

		for (int32 t = 0; t < Ticks.Num(); t += 2)
		{
			TArray<FVector2f> SingleTick = { Ticks[t], Ticks[t + 1] };
			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId + 2,
				AllottedGeometry.ToPaintGeometry(),
				SingleTick,
				ESlateDrawEffect::None,
				ReticleColor.CopyWithNewOpacity(0.65f),
				true,
				1.5f
			);
		}

		// Center Dot
		TArray<FVector2f> CenterDot = {
			FVector2f(Center.X - 1.5f, Center.Y),
			FVector2f(Center.X + 1.5f, Center.Y)
		};
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 2,
			AllottedGeometry.ToPaintGeometry(),
			CenterDot,
			ESlateDrawEffect::None,
			ReticleColor.CopyWithNewOpacity(0.90f),
			true,
			3.0f
		);
	}

	// --- 4. Directional Pointer Arrow (Star Citizen Style) ---
	if (bShowDirectionPointer && HoveredSegmentIndex >= 0)
	{
		const float TargetAngleDeg = StartAngleOffset + HoveredSegmentIndex * AngleStep;
		const float AngleRad = FMath::DegreesToRadians(TargetAngleDeg);
		const FVector2D Dir(FMath::Cos(AngleRad), FMath::Sin(AngleRad));
		const FVector2D Ortho(-Dir.Y, Dir.X);

		const float TipDist = InnerRadius - 8.0f;
		const float BaseDist = InnerRadius - 18.0f;
		const float HalfWidth = 5.0f;

		const FVector2f Tip(Center + Dir * TipDist);
		const FVector2f Base1(Center + Dir * BaseDist + Ortho * HalfWidth);
		const FVector2f Base2(Center + Dir * BaseDist - Ortho * HalfWidth);

		TArray<FVector2f> ArrowPoints = { Tip, Base1, Base2, Tip };
		FSlateDrawElement::MakeLines(
			OutDrawElements,
			LayerId + 3,
			AllottedGeometry.ToPaintGeometry(),
			ArrowPoints,
			ESlateDrawEffect::None,
			PointerColor,
			true,
			1.5f
		);
	}

	// Paint child UMG widgets (Icons, Labels, Center text) on top
	return Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId + 4, InWidgetStyle, bParentEnabled);
}

void UPrismRadialMenu::DrawAnnularSector(
	FSlateWindowElementList& OutDrawElements,
	int32 LayerId,
	const FGeometry& AllottedGeometry,
	const FVector2D& Center,
	float InInnerR,
	float InOuterR,
	float StartAngleDeg,
	float EndAngleDeg,
	const FLinearColor& FillColor,
	const FLinearColor& OutlineColor,
	float OutlineThickness) const
{
	// 1. Procedural Solid Sector Fill via Concentric Arc Bands
	if (FillColor.A > 0.005f)
	{
		const float StepR = 2.0f;
		const int32 NumRadialBands = FMath::Max(1, FMath::CeilToInt((InOuterR - InInnerR) / StepR));
		const int32 ArcSteps = FMath::Clamp(FMath::RoundToInt((EndAngleDeg - StartAngleDeg) / 2.5f), 8, 36);
		const float AngleStep = (EndAngleDeg - StartAngleDeg) / static_cast<float>(ArcSteps);

		TArray<FVector2f> BandPoints;
		BandPoints.Reserve(ArcSteps + 1);

		for (int32 b = 0; b <= NumRadialBands; ++b)
		{
			const float R = FMath::Min(InInnerR + b * StepR, InOuterR);
			BandPoints.Reset();

			for (int32 i = 0; i <= ArcSteps; ++i)
			{
				const float AngleRad = FMath::DegreesToRadians(StartAngleDeg + i * AngleStep);
				BandPoints.Add(FVector2f(Center.X + R * FMath::Cos(AngleRad), Center.Y + R * FMath::Sin(AngleRad)));
			}

			FSlateDrawElement::MakeLines(
				OutDrawElements,
				LayerId,
				AllottedGeometry.ToPaintGeometry(),
				BandPoints,
				ESlateDrawEffect::None,
				FillColor,
				true,
				StepR + 0.6f
			);
		}
	}

	// 2. Closed Outline around the Annular Sector
	const int32 OutlineSteps = FMath::Clamp(FMath::RoundToInt((EndAngleDeg - StartAngleDeg) / 2.5f), 8, 36);
	const float OutlineAngleStep = (EndAngleDeg - StartAngleDeg) / static_cast<float>(OutlineSteps);

	TArray<FVector2f> OutlinePoints;
	OutlinePoints.Reserve((OutlineSteps + 1) * 2 + 2);

	// Outer arc
	for (int32 i = 0; i <= OutlineSteps; ++i)
	{
		const float AngleRad = FMath::DegreesToRadians(StartAngleDeg + i * OutlineAngleStep);
		OutlinePoints.Add(FVector2f(Center.X + InOuterR * FMath::Cos(AngleRad), Center.Y + InOuterR * FMath::Sin(AngleRad)));
	}
	// Inner arc (reversed)
	for (int32 i = OutlineSteps; i >= 0; --i)
	{
		const float AngleRad = FMath::DegreesToRadians(StartAngleDeg + i * OutlineAngleStep);
		OutlinePoints.Add(FVector2f(Center.X + InInnerR * FMath::Cos(AngleRad), Center.Y + InInnerR * FMath::Sin(AngleRad)));
	}

	// Close loop
	if (OutlinePoints.Num() > 0)
	{
		const FVector2f FirstPoint(OutlinePoints[0].X, OutlinePoints[0].Y);
		OutlinePoints.Add(FirstPoint);
	}

	FSlateDrawElement::MakeLines(
		OutDrawElements,
		LayerId + 1,
		AllottedGeometry.ToPaintGeometry(),
		OutlinePoints,
		ESlateDrawEffect::None,
		OutlineColor,
		true,
		OutlineThickness
	);
}
