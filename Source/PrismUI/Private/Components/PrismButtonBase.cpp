#include "Components/PrismButtonBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/TextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"


UPrismButtonBase::UPrismButtonBase(const FObjectInitializer &ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsFocusable(true);
}


// --- Lifecycle ---

void UPrismButtonBase::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply
UPrismButtonBase::NativeOnMouseButtonDown(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent)
{
	if (GetCurrentState() == EPrismWidgetState::Disabled)
		return FReply::Unhandled();

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		SetWidgetState(EPrismWidgetState::Pressed);
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return FReply::Unhandled();
}

FReply
UPrismButtonBase::NativeOnMouseButtonUp(const FGeometry &InGeometry, const FPointerEvent &InMouseEvent)
{
	if (GetCurrentState() == EPrismWidgetState::Disabled)
		return FReply::Unhandled();

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (GetCurrentState() == EPrismWidgetState::Pressed)
			HandleClick();

		SetWidgetState(InGeometry.IsUnderLocation(InMouseEvent.GetScreenSpacePosition())
		    ? EPrismWidgetState::Hovered : EPrismWidgetState::Normal);

		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

void UPrismButtonBase::NativeOnMouseEnter(const FGeometry &InMyGeometry, const FPointerEvent &InMouseEvent)
{
	Super::NativeOnMouseEnter(InMyGeometry, InMouseEvent);

	SetWidgetState(EPrismWidgetState::Hovered);
	const FPrismUIWidgetStyle &Style = GetEffectiveStyle();

	if (Style.HoverSound)
		UGameplayStatics::PlaySound2D(this, Style.HoverSound);
}

void UPrismButtonBase::NativeOnMouseLeave(const FPointerEvent &InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	SetWidgetState(EPrismWidgetState::Normal);
}

FReply UPrismButtonBase::NativeOnKeyDown(const FGeometry &InMyGeometry, const FKeyEvent &InKeyEvent)
{
	if (GetCurrentState() == EPrismWidgetState::Disabled)
		return FReply::Unhandled();

	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Virtual_Accept || Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		HandleClick();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InMyGeometry, InKeyEvent);
}


// --- Logic ---

UPrismButtonBase* UPrismButtonBase::SetRole(FName InRole)
{
	UPrismWidgetBase::SetRole(InRole);
	return this;
}

UPrismButtonBase* UPrismButtonBase::SetStyleOverride(UPrismUIStyle* InStyle)
{
	UPrismWidgetBase::SetStyleOverride(InStyle);
	return this;
}

void UPrismButtonBase::SetText(FText InText)
{
	ButtonText = InText;
	BuildDefaultLayout();
	RefreshStyle();
}

void UPrismButtonBase::SetIcon(UTexture2D* InIcon)
{
	IconTexture = InIcon;
	BuildDefaultLayout();
	RefreshStyle();
}

void UPrismButtonBase::SetIsVertical(bool bInIsVertical)
{
	bIsVertical = bInIsVertical;
	BuildDefaultLayout();
	RefreshStyle();
}

void UPrismButtonBase::HandleClick()
{
	OnClicked.Broadcast(this);

	const FPrismUIWidgetStyle &Style = GetEffectiveStyle();
	if (Style.ClickSound)
		UGameplayStatics::PlaySound2D(this, Style.ClickSound);

	UE_LOG(LogTemp, Display, TEXT("PrismUI: Button Clicked - %s"), *GetName());
}

void UPrismButtonBase::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
		return;

	// 1. Background Border
	if (!BackgroundBorder)
	{
		BackgroundBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
		BackgroundBorder->SetVisibility(ESlateVisibility::Visible);

		if (!Tree->RootWidget)
			Tree->RootWidget = BackgroundBorder;
	}

	if (BackgroundBorder)
	{
		BackgroundBorder->SetPadding(ContentPadding);
	}

	// 2. Content Layout Stacking
	if (bIsVertical)
	{
		if (!ContentVBox)
			ContentVBox = Tree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		
		if (ContentVBox && ContentVBox->GetParent() != BackgroundBorder)
		{
			if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(BackgroundBorder->AddChild(ContentVBox)))
			{
				BorderSlot->SetHorizontalAlignment(HAlign_Center);
				BorderSlot->SetVerticalAlignment(VAlign_Center);
			}
		}

		if (ContentHBox)
			ContentHBox->SetVisibility(ESlateVisibility::Collapsed);
		
		if (ContentVBox)
			ContentVBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else
	{
		if (!ContentHBox)
			ContentHBox = Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());

		if (ContentHBox && ContentHBox->GetParent() != BackgroundBorder)
		{
			if (UBorderSlot* BorderSlot = Cast<UBorderSlot>(BackgroundBorder->AddChild(ContentHBox)))
			{
				BorderSlot->SetHorizontalAlignment(HAlign_Center);
				BorderSlot->SetVerticalAlignment(VAlign_Center);
			}
		}

		if (ContentVBox)
			ContentVBox->SetVisibility(ESlateVisibility::Collapsed);
		
		if (ContentHBox)
			ContentHBox->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	UPanelWidget* CurrentRoot = bIsVertical ? Cast<UPanelWidget>(ContentVBox) : Cast<UPanelWidget>(ContentHBox);

	// 3. Icon
	if (IconTexture && !IconImage)
	{
		IconImage = Tree->ConstructWidget<UImage>(UImage::StaticClass());
		IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (CurrentRoot)
			CurrentRoot->AddChild(IconImage);
	}
	
	if (IconImage)
	{
		IconImage->SetBrushFromTexture(IconTexture);
		IconImage->SetDesiredSizeOverride(IconSize);
		IconImage->SetVisibility(IconTexture ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// 4. Label
	if (!ButtonText.IsEmpty() && !TextBlock)
	{
		TextBlock = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		TextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
		if (CurrentRoot)
			CurrentRoot->AddChild(TextBlock);
	}

	if (TextBlock)
	{
		TextBlock->SetText(ButtonText);
		TextBlock->SetVisibility(ButtonText.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
        
		// Dynamic Spacing
		if (IconImage)
		{
			if (bIsVertical)
			{
				if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(TextBlock->Slot))
					VSlot->SetPadding(FMargin(0, 8, 0, 0));
			}
			else
			{
				if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(TextBlock->Slot))
					HSlot->SetPadding(FMargin(8, 0, 0, 0));
			}
		}
	}
}

void UPrismButtonBase::OnStateChanged(EPrismWidgetState InNewState)
{
	bIsHovered = (InNewState == EPrismWidgetState::Hovered || InNewState == EPrismWidgetState::Pressed);
	bIsPressed = (InNewState == EPrismWidgetState::Pressed);

	Super::OnStateChanged(InNewState);
}

void UPrismButtonBase::OnStyleApplied(const FPrismUIWidgetStyle &InStyleData)
{
	Super::OnStyleApplied(InStyleData);
}
