#pragma once
#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismRadialMenu.generated.h"

class UCanvasPanel;
class UImage;
class UTextBlock;
class UCanvasPanelSlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrismRadialSegmentSelected, FName, SegmentID);

USTRUCT(BlueprintType)
struct FPrismRadialSegment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor Color = FLinearColor::White;

	// Internal
	TObjectPtr<UImage> IconWidget = nullptr;
	TObjectPtr<UTextBlock> LabelWidget = nullptr;
};

UCLASS()
class PRISMUI_API UPrismRadialMenu : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UPrismRadialMenu(const FObjectInitializer& ObjectInitializer);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void AddSegment(FName InID, FText InLabel, UTexture2D* InIcon, FLinearColor InColor);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void ClearSegments();

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	void OpenMenu();

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Radial Menu")
	FName CloseMenuAndGetSelection();

	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Radial Menu")
	FOnPrismRadialSegmentSelected OnSegmentSelected;

protected:
	virtual void BuildDefaultLayout() override;

	UPROPERTY(EditAnywhere, Category = "Config")
	float Radius = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Config")
	float InnerDeadZone = 20.0f;

	TArray<FPrismRadialSegment> Segments;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> MainCanvas;

	UPROPERTY()
	TObjectPtr<UImage> BackgroundBlur;

	UPROPERTY()
	TObjectPtr<UImage> CenterHighlight;

	int32 HoveredSegmentIndex = -1;
	FVector2D MouseStartLocation = FVector2D::ZeroVector;

	void UpdateHoverState(int32 NewHoverIndex);
};
