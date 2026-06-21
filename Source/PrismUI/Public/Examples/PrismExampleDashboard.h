// Copyright (c) Bixboy, 2026. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismExampleDashboard.generated.h"

class UVerticalBox;
class UPrismButtonBase;

/**
 * UPrismExampleDashboard
 * A demonstration of a complex UI assembly using PrismUI.
 * Shows how to:
 * 1. Mix different widget roles (Primary, Danger, Success).
 * 2. Dynamically populate a list of widgets.
 * 3. Handle child widget events in a centralized manner.
 */
UCLASS()
class PRISMUI_API UPrismExampleDashboard : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UPrismExampleDashboard(const FObjectInitializer& ObjectInitializer);

protected:
	// --- UI Construction ---
	virtual void NativeConstruct() override;
	
	/** Programmatically builds the dashboard layout if not using a Blueprint. */
	virtual void BuildDefaultLayout() override;

	// --- Component Binding ---
	UPROPERTY(BlueprintReadOnly, Category = "Dashboard | Components", meta = (BindWidget))
	TObjectPtr<UVerticalBox> ActionList = nullptr;

private:
	/** Populates the dashboard with example buttons. */
	void InitializeActions();

	/** Callback for any button click in the dashboard. */
	UFUNCTION()
	void HandleActionClicked(UPrismButtonBase* InButton);

	UPROPERTY(EditAnywhere, Category = "Dashboard | Config")
	TSubclassOf<UPrismButtonBase> ButtonClass;
};
