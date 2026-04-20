#include "Components/PrismVectorInputBase.h"
#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/Image.h"


UPrismVectorInputBase::UPrismVectorInputBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UPrismVectorInputBase::NativeDestruct()
{
	if (TextInputX)
		TextInputX->OnTextCommitted.RemoveAll(this);
	
	if (TextInputY)
		TextInputY->OnTextCommitted.RemoveAll(this);

	Super::NativeDestruct();
}

void UPrismVectorInputBase::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
		return;

	if (!RootBox)
	{
		RootBox = Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		Tree->RootWidget = RootBox;
	}

	auto CreateInputPair = [&](const FString& LabelText, TObjectPtr<UEditableTextBox>& OutTextBox, TObjectPtr<UBorder>& OutBorder, TObjectPtr<UTextBlock>& OutLabel, TObjectPtr<UImage>& OutIcon)
	{
		TObjectPtr<UHorizontalBox> PairBox = Tree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		if (UHorizontalBoxSlot* PairSlot = RootBox->AddChildToHorizontalBox(PairBox))
		{
			PairSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			PairSlot->SetPadding(FMargin(0.f, 0.f, 4.f, 0.f));
		}

		// Label
		OutLabel = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		OutLabel->SetText(FText::FromString(LabelText));
		if (UHorizontalBoxSlot* LabelSlot = PairBox->AddChildToHorizontalBox(OutLabel))
		{
			LabelSlot->SetVerticalAlignment(VAlign_Center);
			LabelSlot->SetPadding(FMargin(0.f, 0.f, 6.f, 0.f));
		}

		// Input Container
		OutBorder = Tree->ConstructWidget<UBorder>(UBorder::StaticClass());
		if (UHorizontalBoxSlot* BorderSlot = PairBox->AddChildToHorizontalBox(OutBorder))
		{
			BorderSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			BorderSlot->SetVerticalAlignment(VAlign_Fill);
		}

		UOverlay* Stack = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
		OutBorder->AddChild(Stack);

		// Editable Text
		OutTextBox = Tree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
		if (UOverlaySlot* BoxSlot = Stack->AddChildToOverlay(OutTextBox))
		{
			BoxSlot->SetPadding(FMargin(8.f, 4.f));
			BoxSlot->SetHorizontalAlignment(HAlign_Fill);
			BoxSlot->SetVerticalAlignment(VAlign_Center);
		}

		// Icon (for Locked/States)
		OutIcon = Tree->ConstructWidget<UImage>(UImage::StaticClass());
		OutIcon->SetVisibility(ESlateVisibility::Collapsed);
		if (UOverlaySlot* IconSlot = Stack->AddChildToOverlay(OutIcon))
		{
			IconSlot->SetHorizontalAlignment(HAlign_Center);
			IconSlot->SetVerticalAlignment(VAlign_Center);
		}
	};

	if (!TextInputX)
		CreateInputPair(TEXT("X"), TextInputX, BorderX, LabelX, IconX);
	
	if (!TextInputY)
		CreateInputPair(TEXT("Y"), TextInputY, BorderY, LabelY, IconY);

	if (TextInputX)
	{
		TextInputX->OnTextCommitted.AddUniqueDynamic(this, &UPrismVectorInputBase::OnXCommitted);
		TextInputX->OnTextChanged.AddUniqueDynamic(this, &UPrismVectorInputBase::OnXTextChanged);
	}

	if (TextInputY)
	{
		TextInputY->OnTextCommitted.AddUniqueDynamic(this, &UPrismVectorInputBase::OnYCommitted);
		TextInputY->OnTextChanged.AddUniqueDynamic(this, &UPrismVectorInputBase::OnYTextChanged);
	}

	SyncTextWidgets();
}

void UPrismVectorInputBase::SetVector(FIntPoint InVector)
{
	if (CurrentVector == InVector)
		return;

	CurrentVector = InVector;
	SyncTextWidgets();
	OnVectorChanged.Broadcast(CurrentVector);
}

void UPrismVectorInputBase::OnXCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (bIsInternalUpdate)
		return;

	int32 Val = FCString::Atoi(*Text.ToString());
	OnComponentChanged(TEXT("X"), Val);
}

void UPrismVectorInputBase::OnYCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (bIsInternalUpdate)
		return;

	int32 Val = FCString::Atoi(*Text.ToString());
	OnComponentChanged(TEXT("Y"), Val);
}

void UPrismVectorInputBase::OnXTextChanged(const FText& Text)
{
	if (bIsInternalUpdate)
		return;

	FString Raw = Text.ToString();
	FString Filtered;
	for (TCHAR C : Raw)
	{
		if (FChar::IsDigit(C))
			Filtered.AppendChar(C);
	}

	if (Raw != Filtered)
	{
		bIsInternalUpdate = true;
		TextInputX->SetText(FText::FromString(Filtered));
		bIsInternalUpdate = false;
	}

	if (!Filtered.IsEmpty())
	{
		int32 Val = FCString::Atoi(*Filtered);
		OnComponentChanged(TEXT("X"), Val);
	}
}

void UPrismVectorInputBase::OnYTextChanged(const FText& Text)
{
	if (bIsInternalUpdate)
		return;

	FString Raw = Text.ToString();
	FString Filtered;
	for (TCHAR C : Raw)
	{
		if (FChar::IsDigit(C)) 
			Filtered.AppendChar(C);
	}

	if (Raw != Filtered)
	{
		bIsInternalUpdate = true;
		TextInputY->SetText(FText::FromString(Filtered));
		bIsInternalUpdate = false;
	}

	if (!Filtered.IsEmpty())
	{
		int32 Val = FCString::Atoi(*Filtered);
		OnComponentChanged(TEXT("Y"), Val);
	}
}

void UPrismVectorInputBase::OnComponentChanged(const FString& ComponentName, int32 NewValue)
{
	int32 Clamped = FMath::Max(1, NewValue);

	if (ComponentName == TEXT("X"))
	{
		CurrentVector.X = Clamped;
	}
	else
	{
		CurrentVector.Y = Clamped;
	}

	OnVectorChanged.Broadcast(CurrentVector);
}

void UPrismVectorInputBase::SyncTextWidgets()
{
	bIsInternalUpdate = true;
	if (TextInputX)
		TextInputX->SetText(FText::AsNumber(CurrentVector.X));
	
	if (TextInputY)
		TextInputY->SetText(FText::AsNumber(CurrentVector.Y));
	
	bIsInternalUpdate = false;
}
