// Copyright (c) Bixboy, 2026. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Core/PrismUITypes.h"
#include "PrismWidgetBase.h"
#include "Components/PanelWidget.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/VerticalBox.h"
#include "Components/HorizontalBox.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"

class UPrismThemeSubsystem;


struct PRISMUI_API FPrismBuilder
{
public:
	explicit FPrismBuilder(UUserWidget* InOwnerWidget, UWidgetTree* InTree);

	// --- Core ---
	
	/** Finishes the building process and returns the Root Widget. */
	UWidget* Build();

	// --- Layout Panels ---

	FPrismBuilder& BeginOverlay();
	FPrismBuilder& BeginVerticalBox();
	FPrismBuilder& BeginHorizontalBox();
	FPrismBuilder& EndPanel();

	// --- Themed Elements ---

	FPrismBuilder& AddBackground(FName InRole = TEXT("Primary"), EPrismColorToken InBgToken = EPrismColorToken::Background);
	FPrismBuilder& AddText(const FText& InText, FName InRole = TEXT("Primary"), EPrismTypographyToken InTypoToken = EPrismTypographyToken::Body, EPrismColorToken InColorToken = EPrismColorToken::TextPrimary);

	// --- Slot Modifiers ---
	
	FPrismBuilder& SetPadding(FMargin InPadding);
	FPrismBuilder& SetHorizontalAlignment(EHorizontalAlignment InHAlign);
	FPrismBuilder& SetVerticalAlignment(EVerticalAlignment InVAlign);
	FPrismBuilder& SetSize(FSlateChildSize InSize);

	// --- Data Binding ---

	/**
	 * Binds the last created widget to an Unreal Multicast Delegate (Event) in a Zero-Tick manner.
	 * The binding is automatically cleaned up when the WidgetBase is destroyed or returned to the pool.
	 * 
	 * Example:
	 * .AddText(...)
	 * .BindEvent(PlayerRef->OnHealthChanged, [](UWidget* W, int32 NewHP) { Cast<UTextBlock>(W)->SetText(...); })
	 */
	template<typename DelegateType, typename LambdaType>
	FPrismBuilder& BindEvent(DelegateType& InEvent, LambdaType InCallback)
	{
		if (!LastCreatedWidget || !OwnerWidget)
		{
			return *this;
		}

		UPrismWidgetBase* PrismOwner = Cast<UPrismWidgetBase>(OwnerWidget);
		if (!PrismOwner)
		{
			return *this;
		}

		UWidget* TargetWidget = LastCreatedWidget;
		TSharedPtr<bool> bIsActive = MakeShared<bool>(true);

		InEvent.AddWeakLambda(PrismOwner, [TargetWidget, InCallback, bIsActive](auto&&... Args)
		{
			if (*bIsActive && IsValid(TargetWidget))
			{
				InCallback(TargetWidget, Forward<decltype(Args)>(Args)...);
			}
		});

		PrismOwner->AddBindingCleanup([bIsActive]()
		{
			*bIsActive = false;
		});

		return *this;
	}

	// --- Getters ---
	UWidget* GetLastCreatedWidget() const { return LastCreatedWidget; }

private:
	UUserWidget* OwnerWidget = nullptr;
	UWidgetTree* Tree = nullptr;
	UWidget* RootWidget = nullptr;
	
	UWidget* LastCreatedWidget = nullptr;
	TArray<UPanelWidget*> PanelStack;

	void AddWidgetToCurrentPanel(UWidget* InWidget);
	FLinearColor ResolveColor(FName InRole, EPrismColorToken InToken) const;
	FSlateFontInfo ResolveTypography(FName InRole, EPrismTypographyToken InToken) const;
};
