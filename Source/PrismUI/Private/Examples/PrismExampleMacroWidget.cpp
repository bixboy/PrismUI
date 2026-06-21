// Copyright (c) Bixboy, 2026. All Rights Reserved.
#include "Examples/PrismExampleMacroWidget.h"
#include "Utilities/PrismUI_Builder.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"

UPrismExampleMacroWidget::UPrismExampleMacroWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	WidgetRole = TEXT("Secondary");
}

void UPrismExampleMacroWidget::BuildDefaultLayout()
{
	Super::BuildDefaultLayout();

	// We access the WidgetTree from the base UUserWidget
	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
		return;

	// 1. Create and set the Root Background
	PRISM_CREATE_ROOT(UBorder, RootBorder);
	MainBackground = RootBorder;

	// 2. Add an Overlay inside the border to handle content alignment
	PRISM_CREATE(UOverlay, RootOverlay);
	RootBorder->AddChild(RootOverlay);

	// 3. Add a Vertical Box for content using the Overlay macro
	PRISM_CREATE(UVerticalBox, MainVBox);
	ContentList = MainVBox;
	
	// Add the VBox to the Root Overlay
	PRISM_ADD_OVERLAY(RootOverlay, MainVBox, HAlign_Fill, VAlign_Fill, FMargin(20.0f));

	// 4. Add a Title using PRISM macros
	PRISM_CREATE(UTextBlock, MyTitle);
	TitleText = MyTitle;
	MyTitle->SetText(FText::FromString(TEXT("SYSTEM DIAGNOSTICS")));
	
	// Add Title to the top of the VBox
	PRISM_ADD_VBOX(MainVBox, MyTitle, ESlateSizeRule::Automatic, HAlign_Center, FMargin(0, 0, 0, 20.0f));

	// 4. Populate with some mock data using a helper
	AddStatusRow(FText::FromString(TEXT("CPU LOAD")), FText::FromString(TEXT("42%")));
	AddStatusRow(FText::FromString(TEXT("MEMORY")), FText::FromString(TEXT("8.4 GB / 16 GB")));
	AddStatusRow(FText::FromString(TEXT("NET LATENCY")), FText::FromString(TEXT("15ms")));
}

void UPrismExampleMacroWidget::OnStyleApplied(const FPrismUIWidgetStyle& InStyleData)
{
	Super::OnStyleApplied(InStyleData);

	// Using the PRISM_SET_SCALE utility macro from PrismUI_Builder.h
	PRISM_SET_SCALE(TitleText, 1.2f);

	if (MainBackground)
	{
		if (const FLinearColor* BgColor = InStyleData.Colors.Find(EPrismColorToken::BackgroundDeep))
		{
			MainBackground->SetBrushColor(*BgColor);
		}
	}

	if (TitleText)
	{
		if (const FSlateFontInfo* FontInfo = InStyleData.Typography.Find(EPrismTypographyToken::H1))
		{
			TitleText->SetFont(*FontInfo);
		}
	}
}

void UPrismExampleMacroWidget::AddStatusRow(const FText& InLabel, const FText& InValue)
{
	if (!ContentList)
		return;

	UWidgetTree* Tree = WidgetTree;

	// Create a Horizontal Box for the row
	PRISM_CREATE(UHorizontalBox, RowHBox);

	// Label Text
	PRISM_CREATE(UTextBlock, LabelTxt);
	LabelTxt->SetText(InLabel);
	PRISM_ADD_HBOX(RowHBox, LabelTxt, ESlateSizeRule::Fill, VAlign_Center, FMargin(5.0f));

	// Value Text
	PRISM_CREATE(UTextBlock, ValueTxt);
	ValueTxt->SetText(InValue);
	PRISM_ADD_HBOX(RowHBox, ValueTxt, ESlateSizeRule::Automatic, VAlign_Center, FMargin(5.0f));

	// Add the row to the main list
	PRISM_ADD_VBOX(ContentList, RowHBox, ESlateSizeRule::Automatic, HAlign_Fill, FMargin(0, 5.0f));
}
