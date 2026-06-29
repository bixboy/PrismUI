#pragma once

#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismExampleMacroWidget.generated.h"

/**
 * MACRO: BIND_PRISM_WIDGET
 * A strategic macro to eliminate boilerplate for optional widget bindings.
 */
#define BIND_PRISM_WIDGET(Type, Name) \
	UPROPERTY(BlueprintReadOnly, Category = "Prism | Components", meta = (BindWidgetOptional)) \
	TObjectPtr<Type> Name = nullptr

/**
 * UPrismExampleMacroWidget
 * Demonstrates advanced C++ UI techniques using macros:
 * 1. Boilerplate-free member declarations.
 * 2. Programmatic UI construction via the Prism Builder API.
 * 3. High-performance, zero-tick layout logic.
 */
UCLASS()
class PRISMUI_API UPrismExampleMacroWidget : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UPrismExampleMacroWidget(const FObjectInitializer& ObjectInitializer);

protected:
	// --- Member Declarations (Boilerplate-free) ---

	BIND_PRISM_WIDGET(class UBorder, MainBackground);
	BIND_PRISM_WIDGET(class UVerticalBox, ContentList);
	BIND_PRISM_WIDGET(class UTextBlock, TitleText);

	// --- Layout Construction ---

	/** 
	 * Overriding BuildDefaultLayout to construct the UI programmatically.
	 * This is where the PRISM_ Builder macros truly shine.
	 */
	virtual void BuildDefaultLayout() override;

	virtual void OnStyleApplied(const FPrismUIWidgetStyle& InStyleData) override;

private:
	/** Internal helper to add a status row via macros. */
	void AddStatusRow(const FText& InLabel, const FText& InValue);
};
