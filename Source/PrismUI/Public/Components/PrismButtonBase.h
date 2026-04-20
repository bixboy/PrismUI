#pragma once
#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismButtonBase.generated.h"

class UNamedSlot;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPrismOnButtonClicked, UPrismButtonBase*, Button);


UCLASS(Abstract)
class PRISMUI_API UPrismButtonBase : public UPrismWidgetBase
{
	GENERATED_BODY()
	
protected:
	UPrismButtonBase(const FObjectInitializer& ObjectInitializer);
	
	// --- Lifecycle ---

	virtual void NativeConstruct() override;
	
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnMouseEnter(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
	
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	
	virtual FReply NativeOnKeyDown(const FGeometry& InMyGeometry, const FKeyEvent& InKeyEvent) override;

public:
	// --- Visuals ---
	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Slots", meta = (BindWidgetOptional))
	TObjectPtr<UNamedSlot> ExtensionSlot = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components", meta = (BindWidgetOptional))
	TObjectPtr<class UBorder> BackgroundBorder = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components", meta = (BindWidgetOptional))
	TObjectPtr<class UHorizontalBox> ContentHBox = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components", meta = (BindWidgetOptional))
	TObjectPtr<class UVerticalBox> ContentVBox = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components", meta = (BindWidgetOptional))
	TObjectPtr<class UTextBlock> TextBlock = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components", meta = (BindWidgetOptional))
	TObjectPtr<class UImage> IconImage = nullptr;

	// --- Events ---
	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Events")
	FPrismOnButtonClicked OnClicked;

	virtual UPrismButtonBase* SetRole(FName InRole) override;
	virtual UPrismButtonBase* SetStyleOverride(UPrismUIStyle* InStyle) override;

	// --- Content Helpers ---
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Content")
	virtual void SetText(FText InText);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Content")
	virtual void SetIcon(UTexture2D* InIcon);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Content")
	virtual void SetIsVertical(bool bInIsVertical);

protected:
	virtual void HandleClick();

	virtual void BuildDefaultLayout() override;

	virtual void OnStyleApplied(const FPrismUIWidgetStyle& InStyleData) override;

	virtual void OnStateChanged(EPrismWidgetState InNewState) override;

	// --- Standard Settings ---
	UPROPERTY(EditAnywhere, Category = "Prism UI | Content")
	FText ButtonText = FText::FromString(TEXT("BUTTON"));

	UPROPERTY(EditAnywhere, Category = "Prism UI | Content")
	TObjectPtr<UTexture2D> IconTexture = nullptr;

	UPROPERTY(EditAnywhere, Category = "Prism UI | Content")
	FVector2D IconSize = FVector2D(16.0f, 16.0f);

	UPROPERTY(EditAnywhere, Category = "Prism UI | Content")
	FMargin ContentPadding = FMargin(12.0f, 6.0f);

	UPROPERTY(EditAnywhere, Category = "Prism UI | Content")
	bool bIsVertical = false;

	bool bIsPressed = false;
	bool bIsHovered = false;
};
