#pragma once
#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismSearchBoxBase.generated.h"

class UEditableTextBox;
class UBorder;
class UImage;
class UButton;
class UHorizontalBox;
struct FFocusEvent;
class FWeakWidgetPath;
class FWidgetPath;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPrismOnSearchChanged, const FText &, SearchText);

UCLASS(Abstract)
class PRISMUI_API UPrismSearchBoxBase : public UPrismWidgetBase
{
  GENERATED_BODY()

public:
  UPrismSearchBoxBase(const FObjectInitializer &ObjectInitializer);

  UPROPERTY(BlueprintAssignable, Category = "Prism UI | Events")
  FPrismOnSearchChanged OnSearchChanged;

  UFUNCTION(BlueprintCallable, Category = "Prism UI | Content")
  void SetSearchText(const FText &InText);

  UFUNCTION(BlueprintPure, Category = "Prism UI | Content")
  FText GetSearchText() const;

  UFUNCTION(BlueprintCallable, Category = "Prism UI | Content")
  void ClearSearch();

  UFUNCTION(BlueprintCallable, Category = "Prism UI | Content")
  void UnfocusSearchBox();

protected:
  virtual void NativeConstruct() override;
  virtual void NativeDestruct() override;
  virtual void BuildDefaultLayout() override;
	
  virtual FReply
  NativeOnMouseButtonDown(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent) override;
	
  virtual FReply
  NativeOnPreviewMouseButtonDown(const FGeometry &InMyGeometry, const FPointerEvent &InMouseEvent) override;
	
  virtual void NativeOnMouseEnter(const FGeometry &InMyGeometry, const FPointerEvent &InMouseEvent) override;
	
  virtual void NativeOnMouseLeave(const FPointerEvent &InMouseEvent) override;
	
  virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;
	
  virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;


protected:
  /** Called when the search box (or its text child) gains or loses focus. */
  virtual void OnFocusChanged(bool bInFocused);

  // --- Internal Callbacks ---
  UFUNCTION()
  void HandleTextChanged(const FText &InText);

  UFUNCTION()
  void HandleTextCommitted(const FText &InText, ETextCommit::Type CommitMethod);

  UFUNCTION()
  void HandleClearClicked();

protected:
  /** Robust internal state tracker to prevent reliance on WeakWidgetPaths
   * during focus loss. */
  bool bIsFocusedInternal = false;

  // --- Internal Components ---
  UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
  TObjectPtr<UBorder> BackgroundBorder = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
  TObjectPtr<UEditableTextBox> SearchTextBox = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
  TObjectPtr<UImage> SearchIcon = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
  TObjectPtr<UButton> ClearButton = nullptr;

  UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
  TObjectPtr<UImage> ClearIcon = nullptr;

  // --- Settings ---
  UPROPERTY(EditAnywhere, Category = "Prism UI | Search")
  FText HintText = NSLOCTEXT("PrismUI", "SearchHint", "Search...");

  UPROPERTY(EditAnywhere, Category = "Prism UI | Search")
  bool bShowSearchIcon = true;

  UPROPERTY(EditAnywhere, Category = "Prism UI | Search")
  FVector2D SearchIconSize = FVector2D(16.f, 16.f);
};
