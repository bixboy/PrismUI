// Copyright (c) Bixboy, 2026. All Rights Reserved.
#include "Examples/PrismSimulatorTest.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Core/PrismUIFactory.h"

void UPrismSimulatorTest::NativeConstruct()
{
	Super::NativeConstruct();
	BuildTestLayout();
}

void UPrismSimulatorTest::BuildTestLayout()
{
	if (!MainContainer)
		return;

	MainContainer->ClearChildren();

	// 1. Native UMG Widget
	UTextBlock* TitleText = NewObject<UTextBlock>(this);
	TitleText->SetText(FText::FromString(TEXT("Mission Control")));
	MainContainer->AddChildToVerticalBox(TitleText);

	// 2. Prism UI Widget (Primary)
	if (UPrismButtonBase* StartBtn = UPrismUIFactory::CreateButton(this, ButtonClass))
	{
		StartBtn->SetRole(TEXT("Primary"));
		StartBtn->SetText(FText::FromString(TEXT("Initiate Launch")));
		MainContainer->AddChildToVerticalBox(StartBtn);
	}

	// 3. Prism UI Widget
	if (UPrismButtonBase* AbortBtn = UPrismUIFactory::CreateButton(this, ButtonClass))
	{
		AbortBtn->SetRole(TEXT("Danger"));
		AbortBtn->SetText(FText::FromString(TEXT("Abort System")));
		MainContainer->AddChildToVerticalBox(AbortBtn);
	}
}
