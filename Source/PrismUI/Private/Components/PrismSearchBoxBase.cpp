#include "Components/PrismSearchBoxBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Framework/Application/SlateApplication.h"


#include "Input/Events.h"
#include "Layout/WidgetPath.h"


UPrismSearchBoxBase::UPrismSearchBoxBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UPrismSearchBoxBase::NativeConstruct()
{
    Super::NativeConstruct();
}

void UPrismSearchBoxBase::NativeDestruct()
{

	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.RemoveAll(this);
		SearchTextBox->OnTextCommitted.RemoveAll(this);
	}
	if (ClearButton)
	{
		ClearButton->OnClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPrismSearchBoxBase::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
		return;

	if (!BackgroundBorder)
	{
		BackgroundBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
		Tree->RootWidget = BackgroundBorder;
	}
    BackgroundBorder->SetVisibility(ESlateVisibility::Visible);

	UOverlay* ContentOverlay = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
    ContentOverlay->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	BackgroundBorder->AddChild(ContentOverlay);

	// 1. Search Icon (Left)
	SearchIcon = Tree->ConstructWidget<UImage>(UImage::StaticClass());
	SearchIcon->SetDesiredSizeOverride(SearchIconSize);
	SearchIcon->SetVisibility(bShowSearchIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	
	if (UOverlaySlot* IconSlot = ContentOverlay->AddChildToOverlay(SearchIcon))
	{
		IconSlot->SetHorizontalAlignment(HAlign_Left);
		IconSlot->SetVerticalAlignment(VAlign_Center);
		IconSlot->SetPadding(FMargin(10.f, 0.f, 0.f, 0.f));
	}

	// 2. Search Text Box
	SearchTextBox = Tree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
	SearchTextBox->SetHintText(HintText);
    SearchTextBox->SetVisibility(ESlateVisibility::Visible);
	
	if (UOverlaySlot* BoxSlot = ContentOverlay->AddChildToOverlay(SearchTextBox))
	{
		BoxSlot->SetHorizontalAlignment(HAlign_Fill);
		BoxSlot->SetVerticalAlignment(VAlign_Fill);
		// Padding left adjusted for the new icon position
		BoxSlot->SetPadding(FMargin(bShowSearchIcon ? 36.f : 12.f, 4.f, 40.f, 4.f));
	}

	// 3. Clear Button (Right)
	ClearButton = Tree->ConstructWidget<UButton>(UButton::StaticClass());
	ClearButton->SetVisibility(ESlateVisibility::Collapsed);
	
	if (UOverlaySlot* ClearSlot = ContentOverlay->AddChildToOverlay(ClearButton))
	{
		ClearSlot->SetHorizontalAlignment(HAlign_Right);
		ClearSlot->SetVerticalAlignment(VAlign_Center);
		ClearSlot->SetPadding(FMargin(0.f, 0.f, 8.f, 0.f));
	}

	ClearIcon = Tree->ConstructWidget<UImage>(UImage::StaticClass());
	ClearIcon->SetDesiredSizeOverride(FVector2D(12.f, 12.f));
	ClearButton->AddChild(ClearIcon);

	// Bind Events
	if (SearchTextBox)
	{
		SearchTextBox->OnTextChanged.AddUniqueDynamic(this, &UPrismSearchBoxBase::HandleTextChanged);
		SearchTextBox->OnTextCommitted.AddUniqueDynamic(this, &UPrismSearchBoxBase::HandleTextCommitted);
	}

	if (ClearButton)
	{
		ClearButton->OnClicked.AddUniqueDynamic(this, &UPrismSearchBoxBase::HandleClearClicked);
	}
}

void UPrismSearchBoxBase::SetSearchText(const FText& InText)
{
	if (SearchTextBox)
	{
		SearchTextBox->SetText(InText);
		HandleTextChanged(InText);
	}
}

FText UPrismSearchBoxBase::GetSearchText() const
{
	return SearchTextBox ? SearchTextBox->GetText() : FText::GetEmpty();
}

void UPrismSearchBoxBase::ClearSearch()
{
	SetSearchText(FText::GetEmpty());
}

void UPrismSearchBoxBase::UnfocusSearchBox()
{
	if (FSlateApplication::IsInitialized())
	{
		// Safely clear focus and return it to the game viewport
		FSlateApplication::Get().SetAllUserFocusToGameViewport(EFocusCause::Cleared);
	}
}

FReply UPrismSearchBoxBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (SearchTextBox)
	{
		SearchTextBox->SetFocus();
	}

	return FReply::Handled();
}

FReply UPrismSearchBoxBase::NativeOnPreviewMouseButtonDown(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
    // Proactive focus steal
    if (SearchTextBox)
    {
        SearchTextBox->SetFocus();
    }
    return FReply::Handled();
}

void UPrismSearchBoxBase::NativeOnMouseEnter(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseEnter(InMyGeometry, InMouseEvent);
    StartVisualTransition(1.0f);
}

void UPrismSearchBoxBase::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
    Super::NativeOnMouseLeave(InMouseEvent);
    StartVisualTransition(0.0f);
}
void UPrismSearchBoxBase::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnAddedToFocusPath(InFocusEvent);
    
    if (!bIsFocusedInternal)
    {
        bIsFocusedInternal = true;
        OnFocusChanged(true);
    }
}

void UPrismSearchBoxBase::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnRemovedFromFocusPath(InFocusEvent);
    
    if (bIsFocusedInternal)
    {
        bIsFocusedInternal = false;
        OnFocusChanged(false);
    }
}

void UPrismSearchBoxBase::HandleTextChanged(const FText& InText)
{
	// Toggle Clear Button Visibility
	if (ClearButton)
	{
		ClearButton->SetVisibility(InText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	OnSearchChanged.Broadcast(InText);
}

void UPrismSearchBoxBase::HandleTextCommitted(const FText& InText, ETextCommit::Type CommitMethod)
{
    // Automatically unfocus when the user hits Enter (commit) or Escape (cancel/clear)
    if (CommitMethod == ETextCommit::OnEnter || CommitMethod == ETextCommit::OnCleared)
    {
        UnfocusSearchBox();
    }
}

void UPrismSearchBoxBase::HandleClearClicked()
{
	ClearSearch();
	if (SearchTextBox)
	{
		SearchTextBox->SetFocus();
	}
}

void UPrismSearchBoxBase::OnFocusChanged(bool bInFocused)
{
    // Override in children
}
