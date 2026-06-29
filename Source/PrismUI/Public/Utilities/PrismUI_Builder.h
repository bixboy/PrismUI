#pragma once

#include "CoreMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/OverlaySlot.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBoxSlot.h"

/** 
 * PRISM UI BUILDER MACROS
 * Designed for Elite C++ Programmatic UI Construction.
 * Minimizes boilerplate while maintaining strictly typed UMG generation.
 */

// --- Construction ---

/** Constructs a widget from the given Class and assigns it to VarName. */
#define PRISM_CREATE(Class, VarName) \
    TObjectPtr<Class> VarName = Tree->ConstructWidget<Class>(Class::StaticClass())

/** Constructs a widget and sets it as the tree root. */
#define PRISM_CREATE_ROOT(Class, VarName) \
    PRISM_CREATE(Class, VarName); \
    Tree->RootWidget = VarName

// --- Nesting & Slots ---

/** Helper to cast a slot and apply settings safely. */
#define PRISM_SLOT_CAST(SlotClass, Parent, Child, VarName) \
    TObjectPtr<SlotClass> VarName = Cast<SlotClass>(Parent->AddChild(Child))

/** Adds a child to an Overlay and sets alignment/padding in one go. */
#define PRISM_ADD_OVERLAY(Parent, Child, HAlign, VAlign, Padding) \
    if (UOverlaySlot* _TmpSlot = Parent->AddChildToOverlay(Child)) { \
        _TmpSlot->SetHorizontalAlignment(HAlign); \
        _TmpSlot->SetVerticalAlignment(VAlign); \
        _TmpSlot->SetPadding(Padding); \
    }

/** Adds a child to an HorizontalBox with Sizing, Vertical Alignment and Padding. */
#define PRISM_ADD_HBOX(Parent, Child, SizeRule, VAlign, Padding) \
    if (UHorizontalBoxSlot* _TmpSlot = Parent->AddChildToHorizontalBox(Child)) { \
        _TmpSlot->SetSize(FSlateChildSize(SizeRule)); \
        _TmpSlot->SetVerticalAlignment(VAlign); \
        _TmpSlot->SetPadding(Padding); \
    }

/** Adds a child to an VerticalBox with Sizing, Horizontal Alignment and Padding. */
#define PRISM_ADD_VBOX(Parent, Child, SizeRule, HAlign, Padding) \
    if (UVerticalBoxSlot* _TmpSlot = Parent->AddChildToVerticalBox(Child)) { \
        _TmpSlot->SetSize(FSlateChildSize(SizeRule)); \
        _TmpSlot->SetHorizontalAlignment(HAlign); \
        _TmpSlot->SetPadding(Padding); \
    }

/** Adds a child to a SizeBox with alignment. */
#define PRISM_ADD_SIZEBOX(Parent, Child, HAlign, VAlign) \
    if (USizeBoxSlot* _TmpSlot = Cast<USizeBoxSlot>(Parent->AddChild(Child))) { \
        _TmpSlot->SetHorizontalAlignment(HAlign); \
        _TmpSlot->SetVerticalAlignment(VAlign); \
    }

// --- Shortcuts ---

/** Quick helper to set Render Scale. */
#define PRISM_SET_SCALE(Widget, Scale) \
    if (Widget) Widget->SetRenderScale(FVector2D(Scale, Scale))

/** Quick helper to set Visibility. */
#define PRISM_SET_VIS(Widget, Vis) \
    if (Widget) Widget->SetVisibility(ESlateVisibility::Vis)
