#pragma once
#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismVectorInputBase.generated.h"

class UEditableTextBox;
class UHorizontalBox;
class UBorder;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPrismOnVectorChanged, FIntPoint, NewVector);


UCLASS(Abstract)
class PRISMUI_API UPrismVectorInputBase : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UPrismVectorInputBase(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "Prism UI | Events")
	FPrismOnVectorChanged OnVectorChanged;

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Content")
	void SetVector(FIntPoint InVector);

	UFUNCTION(BlueprintPure, Category = "Prism UI | Content")
	FORCEINLINE FIntPoint GetVector() const { return CurrentVector; }

protected:
	virtual void NativeDestruct() override;

	virtual void BuildDefaultLayout() override;

	virtual void OnComponentChanged(const FString& ComponentName, int32 NewValue);

private:
	UFUNCTION()
	void OnXCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnYCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void OnXTextChanged(const FText& Text);

	UFUNCTION()
	void OnYTextChanged(const FText& Text);

	void SyncTextWidgets();

protected:
	// --- Internal Components ---
	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UHorizontalBox> RootBox = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UEditableTextBox> TextInputX = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UEditableTextBox> TextInputY = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UBorder> BorderX = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UBorder> BorderY = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UTextBlock> LabelX = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UTextBlock> LabelY = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UImage> IconX = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Prism UI | Components")
	TObjectPtr<UImage> IconY = nullptr;

	// --- State ---
	UPROPERTY(EditAnywhere, Category = "Prism UI | Initial")
	FIntPoint CurrentVector = FIntPoint(1, 1);

	bool bIsInternalUpdate = false;
};
