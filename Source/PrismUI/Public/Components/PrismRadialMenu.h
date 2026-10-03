// Copyright (c) Bixboy, 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismRadialMenu.generated.h"

class UCanvasPanel;
class UImage;
class UTextBlock;
class UOverlay;
class UVerticalBox;
class USizeBox;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrismRadialSegmentSelected, FName, SegmentID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPrismRadialSegmentHovered, FName, SegmentID, int32, SegmentIndex);

/**
 * Data and visual descriptor for a single radial segment / sector.
 */
USTRUCT(BlueprintType)
struct PRISMUI_API FPrismRadialSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	FName ID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	FText Subtitle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	TObjectPtr<UTexture2D> Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	FLinearColor Color = FLinearColor(0.0f, 0.85f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Segment")
	bool bIsEnabled = true;

	// Internal UI Widgets & Smooth Animation State
	UPROPERTY()
	TObjectPtr<UImage> IconWidget = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> LabelWidget = nullptr;

	float HoverProgress = 0.0f;
};

/**
 * AAA Sci-Fi Holographic Interaction & Radial Wheel Menu.
 * Features dynamic sector subdivision, procedural vector drawing (custom annular wedges,
 * outer/inner glow rims, tactical reticles), smooth hover interpolation, and center telemetry HUD.
 */
UCLASS()
class PRISMUI_API UPrismRadialMenu : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UPrismRadialMenu(const FObjectInitializer& ObjectInitializer);

	// --- Lifecycle & Painting ---
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	// --- Segment Operations ---

	/** Adds a standard segment (backward-compatible) */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void AddSegment(FName InID, FText InLabel, UTexture2D* InIcon, FLinearColor InColor);

	/** Adds a segment with title and subtitle metadata */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void AddSegmentEx(FName InID, FText InLabel, FText InSubtitle, UTexture2D* InIcon, FLinearColor InColor = FLinearColor(0.0f, 0.85f, 1.0f, 1.0f));

	/** Clears all registered segments */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void ClearSegments();

	/** Sets whether a specific segment is interactable */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SetSegmentEnabled(int32 InIndex, bool bInEnabled);

	/** Updates the data of an existing segment */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SetSegmentData(int32 InIndex, FName InID, FText InLabel, FText InSubtitle, UTexture2D* InIcon, FLinearColor InColor);

	// --- Dynamic Subdivision & Dimension Controls ---

	/**
	 * Sets the subdivision count.
	 * If 0: automatically subdivides 360 degrees based on the number of registered segments.
	 * If > 0: enforces exactly InCount equal sectors (e.g. 4 for 4 weapon quadrants).
	 */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SetSubdivisionCount(int32 InCount);

	/** Returns the effective number of sectors currently drawn */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Radial Menu")
	int32 GetEffectiveSubdivisionCount() const;

	/** Sets the inner and outer radius of the annular wheel */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SetWheelRadii(float InInnerRadius, float InOuterRadius);

	/** Sets the angular gap between adjacent sectors (in degrees) */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SetSeparatorGapAngle(float InGapDegrees);

	/** Sets the start angle offset in degrees (default -90 so slice 0 is at top) */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SetStartAngleOffset(float InAngleDegrees);

	/** Sets the outward translation expansion of the active slice when hovered */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SetHoverExpansionDistance(float InDistance);

	/** Manually updates the central title and subtitle display */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SetCenterInfo(const FText& InTitle, const FText& InSubtitle);

	// --- Menu Lifecycle & Selection ---

	/** Opens the radial menu and initiates smooth holographic entrance */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	virtual void OpenMenu();

	/** Closes the menu and returns the selected segment ID */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	virtual FName CloseMenuAndGetSelection();

	/** Programmatically selects a segment by its index */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void SelectSegmentByIndex(int32 InIndex);

	/** Returns the currently hovered segment index (-1 if none) */
	UFUNCTION(BlueprintPure, Category = "Prism UI | Radial Menu")
	int32 GetHoveredSegmentIndex() const { return HoveredSegmentIndex; }

	/** Broadcast when a segment is confirmed/selected */
	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Radial Menu")
	FOnPrismRadialSegmentSelected OnSegmentSelected;

	/** Broadcast when the cursor hovers over a new segment */
	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Radial Menu")
	FOnPrismRadialSegmentHovered OnSegmentHovered;

protected:
	virtual void BuildDefaultLayout() override;

	// --- Geometry Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	float Radius = 220.0f; // Outer radius of the wheel

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	float InnerRadius = 100.0f; // Inner radius (deadzone / central display cutout)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	float InnerDeadZone = 40.0f; // Minimum mouse distance to activate selection

	/**
	 * Sector subdivision count.
	 * 0 = Auto (subdivides by Segments.Num()).
	 * >= 2 = Fixed number of sectors (e.g., 4 for Primary, Secondary, Melee, Unarmed).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry", meta = (ClampMin = "0", ClampMax = "32"))
	int32 SubdivisionCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	float SeparatorGapAngle = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	float StartAngleOffset = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	float HoverExpansionDistance = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	FVector2D IconDimensions = FVector2D(64.0f, 44.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	bool bShowLabelsInSectors = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	bool bShowCenterReticle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Geometry")
	bool bShowDirectionPointer = true;

	// --- Holographic Sci-Fi Aesthetics ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Visuals")
	FLinearColor DefaultSectorFillColor = FLinearColor(0.015f, 0.025f, 0.038f, 0.78f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Visuals")
	FLinearColor DefaultSectorOutlineColor = FLinearColor(0.0f, 0.75f, 1.0f, 0.28f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Visuals")
	FLinearColor ActiveSectorFillColor = FLinearColor(0.0f, 0.75f, 0.95f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Visuals")
	FLinearColor ActiveSectorOutlineColor = FLinearColor(0.4f, 0.95f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Visuals")
	FLinearColor EmptySectorFillColor = FLinearColor(0.008f, 0.012f, 0.02f, 0.40f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Visuals")
	FLinearColor ReticleColor = FLinearColor(0.0f, 0.75f, 1.0f, 0.45f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Visuals")
	FLinearColor PointerColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.95f);

	// --- Segments & Dynamic Widgets ---

	UPROPERTY()
	TArray<FPrismRadialSegment> Segments;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> MainCanvas = nullptr;

	UPROPERTY()
	TObjectPtr<UImage> BackgroundBlur = nullptr;

	UPROPERTY()
	TObjectPtr<UImage> CenterHighlight = nullptr;

	UPROPERTY()
	TObjectPtr<UOverlay> CenterOverlay = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> CenterTitleText = nullptr;

	UPROPERTY()
	TObjectPtr<UTextBlock> CenterSubtitleText = nullptr;

	// --- Internal State ---

	int32 HoveredSegmentIndex = -1;
	FVector2D MouseStartLocation = FVector2D::ZeroVector;
	float OpenAnimProgress = 1.0f;
	bool bIsOpening = false;

	virtual void UpdateHoverState(int32 NewHoverIndex);
	virtual void RebuildSectorWidgets();

	float GetOuterRadius() const { return FMath::Max(Radius, InnerRadius + 20.0f); }

	virtual void DrawAnnularSector(
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
		float OutlineThickness
	) const;
};
