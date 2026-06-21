// Copyright (c) Bixboy, 2026. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateTypes.h"
#include "Animation/WidgetAnimation.h"
#include "PrismUITypes.generated.h"

/**
 * EPrismWidgetState
 * Defines the standard interaction states for Prism widgets.
 */
UENUM(BlueprintType)
enum class EPrismWidgetState : uint8
{
	Normal,
	Hovered,
	Pressed,
	Disabled,
	Selected
};

/**
 * EPrismColorToken
 * Semantic design tokens for colors.
 */
UENUM(BlueprintType)
enum class EPrismColorToken : uint8
{
	Background,
	BackgroundDeep,
	Surface,
	SurfaceHighlight,
	Outline,
	PrimaryAccent,
	SecondaryAccent,
	Danger,
	Success,
	TextPrimary,
	TextSecondary,
	Transparent
};

/**
 * EPrismTypographyToken
 * Semantic design tokens for text styling.
 */
UENUM(BlueprintType)
enum class EPrismTypographyToken : uint8
{
	H1,
	H2,
	H3,
	Body,
	Caption,
	Label
};

USTRUCT(BlueprintType)
struct PRISMUI_API FPrismUIWidgetStyle
{
	GENERATED_BODY()

	// --- Theme Colors ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme | Colors")
	TMap<EPrismColorToken, FLinearColor> Colors;

	// --- Typography ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Theme | Typography")
	TMap<EPrismTypographyToken, FSlateFontInfo> Typography;

	// --- AAA Brushes ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brushes")
	FButtonStyle CoreBrushes;

	// --- Shapes ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shapes")
	FVector4 CornerRadii = FVector4(8.f, 8.f, 8.f, 8.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shapes")
	FMargin ContentPadding = FMargin(10.f, 5.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shapes")
	float OutlineThickness = 1.0f;

	// --- Visuals & Animation ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TMap<EPrismWidgetState, TObjectPtr<UWidgetAnimation>> StateAnimations;

	// --- Sounds ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> HoverSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> ClickSound = nullptr;
};
