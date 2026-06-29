#include "Utilities/PrismBuilder.h"
#include "Subsystems/PrismUIThemeSubsystem.h"
#include "Engine/LocalPlayer.h"

FPrismBuilder::FPrismBuilder(UUserWidget* InOwnerWidget, UWidgetTree* InTree)
	: OwnerWidget(InOwnerWidget), Tree(InTree)
{
	check(OwnerWidget);
	check(Tree);
}

UWidget* FPrismBuilder::Build()
{
	if (RootWidget && Tree->RootWidget == nullptr)
	{
		Tree->RootWidget = RootWidget;
	}
	return RootWidget;
}

void FPrismBuilder::AddWidgetToCurrentPanel(UWidget* InWidget)
{
	LastCreatedWidget = InWidget;

	if (PanelStack.Num() > 0)
	{
		PanelStack.Top()->AddChild(InWidget);
	}
	else if (!RootWidget)
	{
		RootWidget = InWidget;
	}
}

FPrismBuilder& FPrismBuilder::BeginOverlay()
{
	UOverlay* NewPanel = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	AddWidgetToCurrentPanel(NewPanel);
	PanelStack.Push(NewPanel);
	return *this;
}

FPrismBuilder& FPrismBuilder::BeginVerticalBox()
{
	UVerticalBox* NewPanel = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	AddWidgetToCurrentPanel(NewPanel);
	PanelStack.Push(NewPanel);
	return *this;
}

FPrismBuilder& FPrismBuilder::BeginHorizontalBox()
{
	UHorizontalBox* NewPanel = Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	AddWidgetToCurrentPanel(NewPanel);
	PanelStack.Push(NewPanel);
	return *this;
}

FPrismBuilder& FPrismBuilder::EndPanel()
{
	if (PanelStack.Num() > 0)
	{
		PanelStack.Pop();
	}
	return *this;
}

FLinearColor FPrismBuilder::ResolveColor(FName InRole, EPrismColorToken InToken) const
{
	if (ULocalPlayer* LP = OwnerWidget->GetOwningLocalPlayer())
	{
		if (UPrismUIThemeSubsystem* Subsystem = LP->GetSubsystem<UPrismUIThemeSubsystem>())
		{
			return Subsystem->GetColorToken(InRole, InToken);
		}
	}
	return FLinearColor::White;
}

FSlateFontInfo FPrismBuilder::ResolveTypography(FName InRole, EPrismTypographyToken InToken) const
{
	if (ULocalPlayer* LP = OwnerWidget->GetOwningLocalPlayer())
	{
		if (UPrismUIThemeSubsystem* Subsystem = LP->GetSubsystem<UPrismUIThemeSubsystem>())
		{
			return Subsystem->GetTypographyToken(InRole, InToken);
		}
	}
	return FSlateFontInfo();
}

FPrismBuilder& FPrismBuilder::AddBackground(FName InRole, EPrismColorToken InBgToken)
{
	UBorder* Border = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Border->SetBrushColor(ResolveColor(InRole, InBgToken));
	AddWidgetToCurrentPanel(Border);
	PanelStack.Push(Border); // Border acts as a panel for 1 child
	return *this;
}

FPrismBuilder& FPrismBuilder::AddText(const FText& InText, FName InRole, EPrismTypographyToken InTypoToken, EPrismColorToken InColorToken)
{
	UTextBlock* TextBlock = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	TextBlock->SetText(InText);
	TextBlock->SetFont(ResolveTypography(InRole, InTypoToken));
	TextBlock->SetColorAndOpacity(ResolveColor(InRole, InColorToken));
	AddWidgetToCurrentPanel(TextBlock);
	return *this;
}

// --- Slot Modifiers ---

FPrismBuilder& FPrismBuilder::SetPadding(FMargin InPadding)
{
	if (!LastCreatedWidget) return *this;

	if (UOverlaySlot* OVSlot = Cast<UOverlaySlot>(LastCreatedWidget->Slot))
		OVSlot->SetPadding(InPadding);
	else if (UVerticalBoxSlot* VBSlot = Cast<UVerticalBoxSlot>(LastCreatedWidget->Slot))
		VBSlot->SetPadding(InPadding);
	else if (UHorizontalBoxSlot* HBSlot = Cast<UHorizontalBoxSlot>(LastCreatedWidget->Slot))
		HBSlot->SetPadding(InPadding);
	else if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(LastCreatedWidget->Slot))
		BorderSlot->SetPadding(InPadding);

	return *this;
}

FPrismBuilder& FPrismBuilder::SetHorizontalAlignment(EHorizontalAlignment InHAlign)
{
	if (!LastCreatedWidget) return *this;

	if (UOverlaySlot* OVSlot = Cast<UOverlaySlot>(LastCreatedWidget->Slot))
		OVSlot->SetHorizontalAlignment(InHAlign);
	else if (UVerticalBoxSlot* VBSlot = Cast<UVerticalBoxSlot>(LastCreatedWidget->Slot))
		VBSlot->SetHorizontalAlignment(InHAlign);
	else if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(LastCreatedWidget->Slot))
		BorderSlot->SetHorizontalAlignment(InHAlign);

	return *this;
}

FPrismBuilder& FPrismBuilder::SetVerticalAlignment(EVerticalAlignment InVAlign)
{
	if (!LastCreatedWidget) return *this;

	if (UOverlaySlot* OVSlot = Cast<UOverlaySlot>(LastCreatedWidget->Slot))
		OVSlot->SetVerticalAlignment(InVAlign);
	else if (UHorizontalBoxSlot* HBSlot = Cast<UHorizontalBoxSlot>(LastCreatedWidget->Slot))
		HBSlot->SetVerticalAlignment(InVAlign);
	else if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(LastCreatedWidget->Slot))
		BorderSlot->SetVerticalAlignment(InVAlign);

	return *this;
}

FPrismBuilder& FPrismBuilder::SetSize(FSlateChildSize InSize)
{
	if (!LastCreatedWidget) return *this;

	if (UVerticalBoxSlot* VBSlot = Cast<UVerticalBoxSlot>(LastCreatedWidget->Slot))
		VBSlot->SetSize(InSize);
	else if (UHorizontalBoxSlot* HBSlot = Cast<UHorizontalBoxSlot>(LastCreatedWidget->Slot))
		HBSlot->SetSize(InSize);

	return *this;
}
