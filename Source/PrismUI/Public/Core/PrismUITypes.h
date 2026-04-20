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

USTRUCT(BlueprintType)
struct PRISMUI_API FPrismUIWidgetStyle
{
	GENERATED_BODY()

	// --- LEGACY Brushes (For Backwards Compatibility) ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brushes (Legacy)")
	FSlateBrush Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brushes (Legacy)")
	FSlateBrush Hovered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brushes (Legacy)")
	FSlateBrush Pressed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brushes (Legacy)")
	FSlateBrush Disabled;

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

	// --- Typography ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	FSlateFontInfo Font;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Typography")
	FLinearColor TextColor = FLinearColor::White;

	// --- Visuals & Animation ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TMap<EPrismWidgetState, TObjectPtr<UWidgetAnimation>> StateAnimations;

	// --- Sounds ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> HoverSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> ClickSound = nullptr;
};
