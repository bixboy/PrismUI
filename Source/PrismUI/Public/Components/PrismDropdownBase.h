#pragma once

#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "Components/ComboBoxString.h"
#include "PrismDropdownBase.generated.h"

class UOverlay;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPrismOnSelectionChanged, const FString&, SelectedItem);

/**
 * UPrismDropdownBase — A robust, Zero-Tick wrapper for UComboBoxString.
 * Handles selection logic, asynchronous state transitions, and custom item generation.
 */
UCLASS(Abstract)
class PRISMUI_API UPrismDropdownBase : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UPrismDropdownBase(const FObjectInitializer& ObjectInitializer);

	// --- Public API ---

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Dropdown")
	virtual void SetOptions(const TMap<FString, UTexture2D*>& InOptions);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Dropdown")
	virtual void SetSelectedOption(const FString& InOption);

	UFUNCTION(BlueprintPure, Category = "Prism UI | Dropdown")
	FString GetSelectedOption() const;

public:
	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Dropdown")
	FPrismOnSelectionChanged OnSelectionChanged;

protected:
	// --- UI Overrides ---
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual bool TickTransitions(float DeltaTime) override;
	virtual void BuildDefaultLayout() override;

	// --- Internal Logic ---
	UFUNCTION()
	virtual void HandleOnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	virtual void HandleOnOpening();

	UFUNCTION()
	virtual UWidget* HandleGenerateWidget(FString Item);

	virtual void UpdateMainDisplay(const FString& SelectedItem);

protected:
	// --- Internal Components ---
	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UOverlay> MainOverlay = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UComboBoxString> InternalComboBox = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UTextBlock> SelectedLabel = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UImage> SelectedIcon = nullptr;

	// --- State ---
	UPROPERTY(EditAnywhere, Category = "Prism UI | Appearance")
	float CornerRadius = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Prism UI | Appearance")
	int32 FontSizeLabel = 10;

	UPROPERTY(EditAnywhere, Category = "Prism UI | Appearance")
	int32 FontSizeDropdown = 9;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UTexture2D>> SelectorOptions;
	
	uint64 LastOpeningFrame = 0;
	bool bIsLayoutBuilt = false;
	bool bIsUpdatingSelection = false;

	float InteractiveAlpha = 0.0f;
	float TargetInteractiveAlpha = 0.0f;
};
