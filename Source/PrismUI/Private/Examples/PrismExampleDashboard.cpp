// Copyright (c) Bixboy, 2026. All Rights Reserved.
#include "Examples/PrismExampleDashboard.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Core/PrismUIFactory.h"

UPrismExampleDashboard::UPrismExampleDashboard(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Dashboards often use a secondary or background role
	WidgetRole = TEXT("Secondary");
}

void UPrismExampleDashboard::NativeConstruct()
{
	Super::NativeConstruct();

	InitializeActions();
}

void UPrismExampleDashboard::BuildDefaultLayout()
{
	Super::BuildDefaultLayout();
	
	// If the designer hasn't provided an ActionList in UMG, we could create one here.
	// For this example, we assume it's bound via BindWidget.
}

void UPrismExampleDashboard::InitializeActions()
{
	if (!ActionList || !ButtonClass)
		return;

	// Clear existing items
	ActionList->ClearChildren();

	// Create a "Confirm" button (Primary Role)
	if (UPrismButtonBase* ConfirmBtn = UPrismUIFactory::CreateButton(this, ButtonClass))
	{
		ConfirmBtn->SetRole(TEXT("Primary"));
		ConfirmBtn->SetText(FText::FromString(TEXT("Confirm Selection")));
		ConfirmBtn->OnClicked.AddDynamic(this, &UPrismExampleDashboard::HandleActionClicked);
		
		if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(ActionList->AddChildToVerticalBox(ConfirmBtn)))
			VSlot->SetPadding(FMargin(0.f, 5.f));
	}

	// Create a "Cancel" button (Danger Role)
	if (UPrismButtonBase* CancelBtn = UPrismUIFactory::CreateButton(this, ButtonClass))
	{
		CancelBtn->SetRole(TEXT("Danger"));
		CancelBtn->SetText(FText::FromString(TEXT("Abort Mission")));
		CancelBtn->OnClicked.AddDynamic(this, &UPrismExampleDashboard::HandleActionClicked);
		
		if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(ActionList->AddChildToVerticalBox(CancelBtn)))
			VSlot->SetPadding(FMargin(0.f, 5.f));
	}
}

void UPrismExampleDashboard::HandleActionClicked(UPrismButtonBase* InButton)
{
	if (!InButton)
		return;

	const FString BtnRole = InButton->WidgetRole.ToString();
	UE_LOG(LogTemp, Display, TEXT("Dashboard: Action triggered with Role [%s]"), *BtnRole);

	// Implement logic based on button role or tag
	if (InButton->WidgetRole == TEXT("Danger"))
	{
		// Critical failure/abort logic
	}
}
