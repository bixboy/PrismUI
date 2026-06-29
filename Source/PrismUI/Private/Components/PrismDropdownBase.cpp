#include "Components/PrismDropdownBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Utilities/PrismUI_Builder.h"

UPrismDropdownBase::UPrismDropdownBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UPrismDropdownBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	BuildDefaultLayout();
}

void UPrismDropdownBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildDefaultLayout();
}

bool UPrismDropdownBase::TickTransitions(float DeltaTime)
{
    // 1. Process base transitions
    bool bBaseTicking = Super::TickTransitions(DeltaTime);

    // 2. Interactive Alpha (Hover/Press detection)
    const bool bIsCurrentlyHovered = IsHovered() || (InternalComboBox && InternalComboBox->IsHovered());
    const bool bIsCurrentlyPressed = InternalComboBox && InternalComboBox->HasMouseCapture();

    TargetInteractiveAlpha = (bIsCurrentlyHovered || bIsCurrentlyPressed) ? 1.0f : 0.0f;
    InteractiveAlpha = FMath::FInterpTo(InteractiveAlpha, TargetInteractiveAlpha, DeltaTime, 12.0f);

    bool bInteractiveTicking = !FMath::IsNearlyEqual(InteractiveAlpha, TargetInteractiveAlpha, 0.001f);

    // 3. Update Visuals
    OnVisualsUpdated(CurrentVisualAlpha);

    return bBaseTicking || bInteractiveTicking;
}

void UPrismDropdownBase::BuildDefaultLayout()
{
	if (bIsLayoutBuilt) return;

	UWidgetTree* Tree = WidgetTree;
	if (!Tree) return;

    PRISM_CREATE_ROOT(UOverlay, Master);
    MainOverlay = Master;

    // 1. ComboBox (The Functional Layer)
    PRISM_CREATE(UComboBoxString, Combo);
    InternalComboBox = Combo;
    PRISM_ADD_OVERLAY(MainOverlay, InternalComboBox, HAlign_Fill, VAlign_Fill, 0.f);

    // 2. Preview Content (The Visual Layer)
    PRISM_CREATE(UHorizontalBox, Preview);
    Preview->SetVisibility(ESlateVisibility::HitTestInvisible);
    PRISM_ADD_OVERLAY(MainOverlay, Preview, HAlign_Fill, VAlign_Center, FMargin(12.f, 8.f));

    PRISM_CREATE(UImage, Icon);
    SelectedIcon = Icon;
    if (UHorizontalBoxSlot* IconSlot = Preview->AddChildToHorizontalBox(SelectedIcon))
    {
        IconSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
        IconSlot->SetVerticalAlignment(VAlign_Center);
        SelectedIcon->SetDesiredSizeOverride(FVector2D(18.f, 18.f));
    }

    PRISM_CREATE(UTextBlock, Label);
    SelectedLabel = Label;
    if (UHorizontalBoxSlot* LabelSlot = Preview->AddChildToHorizontalBox(SelectedLabel))
    {
        LabelSlot->SetVerticalAlignment(VAlign_Center);
        FSlateFontInfo Font = SelectedLabel->GetFont();
        Font.Size = FontSizeLabel;
        SelectedLabel->SetFont(Font);
    }

    // Bind Events
    if (InternalComboBox && !InternalComboBox->OnSelectionChanged.IsBound())
    {
        InternalComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPrismDropdownBase::HandleOnSelectionChanged);
        InternalComboBox->OnOpening.AddUniqueDynamic(this, &UPrismDropdownBase::HandleOnOpening);
        InternalComboBox->OnGenerateWidgetEvent.BindDynamic(this, &UPrismDropdownBase::HandleGenerateWidget);
    }

	bIsLayoutBuilt = true;
}

void UPrismDropdownBase::SetOptions(const TMap<FString, UTexture2D*>& InOptions)
{
	SelectorOptions.Empty();
	for (const auto& KVP : InOptions)
	{
		SelectorOptions.Add(KVP.Key, KVP.Value);
	}

	if (InternalComboBox)
	{
		InternalComboBox->ClearOptions();
		for (const auto& KVP : SelectorOptions)
		{
			InternalComboBox->AddOption(KVP.Key);
		}
	}
}

void UPrismDropdownBase::SetSelectedOption(const FString& InOption)
{
	if (InternalComboBox)
	{
		bIsUpdatingSelection = true;
		InternalComboBox->SetSelectedOption(InOption);
		UpdateMainDisplay(InOption);
		bIsUpdatingSelection = false;
	}
}

FString UPrismDropdownBase::GetSelectedOption() const
{
	return InternalComboBox ? InternalComboBox->GetSelectedOption() : TEXT("");
}

void UPrismDropdownBase::HandleOnOpening()
{
	LastOpeningFrame = GFrameCounter;
    RequestTransitionTick();
}

void UPrismDropdownBase::HandleOnSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	UpdateMainDisplay(SelectedItem);
	if (!bIsUpdatingSelection)
	{
		OnSelectionChanged.Broadcast(SelectedItem);
	}
}

UWidget* UPrismDropdownBase::HandleGenerateWidget(FString Item)
{
	UWidgetTree* Tree = WidgetTree;
    // Minimalist item generation logic
	if (!Tree) return nullptr;

	if (GFrameCounter - LastOpeningFrame > 5)
	{
		PRISM_CREATE(USizeBox, Empty);
		Empty->SetHeightOverride(0.f);
		PRISM_SET_VIS(Empty, Collapsed);
		return Empty;
	}

	PRISM_CREATE(UBorder, RowContainer);
	RowContainer->SetBrushColor(FLinearColor::Transparent);
	RowContainer->SetPadding(FMargin(12.f, 8.f));

	PRISM_CREATE(UHorizontalBox, RowBox);
	RowContainer->AddChild(RowBox);
    
	TObjectPtr<UTexture2D>* IconPtr = SelectorOptions.Find(Item);
	if (IconPtr && *IconPtr)
	{
		PRISM_CREATE(USizeBox, IconBox);
		IconBox->SetWidthOverride(20.f);
		IconBox->SetHeightOverride(20.f);

		PRISM_CREATE(UImage, ItemIcon);
		ItemIcon->SetBrushFromTexture(*IconPtr);        
		IconBox->AddChild(ItemIcon);

        PRISM_ADD_HBOX(RowBox, IconBox, ESlateSizeRule::Automatic, VAlign_Center, FMargin(0.f, 0.f, 10.f, 0.f));
	}

	PRISM_CREATE(UTextBlock, ItemLabel);
	ItemLabel->SetText(FText::FromString(Item));
	ItemLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White)); 
	
	FSlateFontInfo Font = ItemLabel->GetFont();
	Font.Size = FontSizeDropdown;
	ItemLabel->SetFont(Font);
    
    PRISM_ADD_HBOX(RowBox, ItemLabel, ESlateSizeRule::Fill, VAlign_Fill, FMargin(0.f));

	return RowContainer;
}

void UPrismDropdownBase::UpdateMainDisplay(const FString& SelectedItem)
{
	if (SelectedLabel) SelectedLabel->SetText(FText::FromString(SelectedItem));

	if (SelectedIcon)
	{
		TObjectPtr<UTexture2D>* IconPtr = SelectorOptions.Find(SelectedItem);
		if (IconPtr && *IconPtr)
		{
			PRISM_SET_VIS(SelectedIcon, HitTestInvisible);
			SelectedIcon->SetBrushFromTexture(*IconPtr);
		}
		else
		{
			PRISM_SET_VIS(SelectedIcon, Collapsed);
		}
	}
}
