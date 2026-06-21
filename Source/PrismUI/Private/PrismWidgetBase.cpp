// Copyright (c) Bixboy, 2026. All Rights Reserved.
#include "PrismWidgetBase.h"
#include "Subsystems/PrismUIThemeSubsystem.h"
#include "Animation/WidgetAnimation.h"
#include "Components/WidgetComponent.h"



UPrismWidgetBase::UPrismWidgetBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsAttributesDirty = true;
}


// --- Lifecycle ---

bool UPrismWidgetBase::Initialize()
{
	bool bResult = Super::Initialize();

	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
	}

	InitializeLayout();

	return bResult;
}

void UPrismWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	InitializeLayout();
	RefreshStyle();
}

void UPrismWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	InitializeLayout();
}

void UPrismWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPrismUIThemeSubsystem* Subsystem = LP->GetSubsystem<UPrismUIThemeSubsystem>())
		{
			if (!Subsystem->OnThemeChanged.IsAlreadyBound(this, &UPrismWidgetBase::HandleThemeChanged))
				Subsystem->OnThemeChanged.AddUniqueDynamic(this, &UPrismWidgetBase::HandleThemeChanged);
		}
	}

	RefreshStyle();
	OnVisualsUpdated(CurrentVisualAlpha);
}

void UPrismWidgetBase::NativeDestruct()
{
	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPrismUIThemeSubsystem* Subsystem = LP->GetSubsystem<UPrismUIThemeSubsystem>())
			Subsystem->OnThemeChanged.RemoveAll(this);
	}

	if (UWorld* World = GetWorld())
		World->GetTimerManager().ClearTimer(TransitionTimer);

	Super::NativeDestruct();
}

UPrismWidgetBase* UPrismWidgetBase::SetStyleOverride(UPrismUIStyle* InStyle)
{
	StyleOverride = InStyle;
	RefreshStyle();
	return this;
}

UPrismWidgetBase* UPrismWidgetBase::SetRole(FName InRole)
{
	WidgetRole = InRole;
	RefreshStyle();
	return this;
}


// --- State Management ---

void UPrismWidgetBase::SetWidgetState(EPrismWidgetState InNewState)
{
	if (CurrentState == InNewState)
		return;

	CurrentState = InNewState;

	const float TargetAlpha = (CurrentState == EPrismWidgetState::Normal) ? 0.0f : 1.0f;
	StartVisualTransition(TargetAlpha, 15.0f);

	OnStateChanged(CurrentState);
}

void UPrismWidgetBase::OnStateChanged(EPrismWidgetState InNewState)
{
	// Virtual stub for child classes
}

void UPrismWidgetBase::HandleThemeChanged(UPrismUIThemeData* InNewTheme)
{
	if (!IsVisible())
	{
		bIsAttributesDirty = true;
		return;
	}
    
	RefreshStyle();
}



#pragma region Visuals and Theme

void UPrismWidgetBase::BuildDefaultLayout()
{
	// Virtual stub for child classes to build their C++ UI hierarchy
}

void UPrismWidgetBase::InitializeLayout()
{
	if (bHasBuiltLayout)
		return;

	bHasBuiltLayout = true;
	BuildDefaultLayout();
}

void UPrismWidgetBase::RefreshStyle()
{
	OnStyleApplied(GetEffectiveStyle());
	
}

const FPrismUIWidgetStyle& UPrismWidgetBase::GetEffectiveStyle() const
{
	if (StyleOverride)
		return StyleOverride->StyleData;

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		if (UPrismUIThemeSubsystem* Subsystem = LP->GetSubsystem<UPrismUIThemeSubsystem>())
		{
			if (UPrismUIStyle* Style = Subsystem->GetStyleForRole(WidgetRole))
				return Style->StyleData;
		}
	}

	return GetDefaultFallbackStyle();
}

void UPrismWidgetBase::OnStyleApplied(const FPrismUIWidgetStyle& InStyleData)
{
	// Extension point for child classes
}

void UPrismWidgetBase::PlayUMGAnimationSafe(UWidgetAnimation* InAnim, float InStartAtTime, int32 InNumLoopsToPlay, EUMGSequencePlayMode::Type InPlayMode, float InPlaybackSpeed)
{
	if (InAnim)
		PlayAnimation(InAnim, InStartAtTime, InNumLoopsToPlay, InPlayMode, InPlaybackSpeed);
}

void UPrismWidgetBase::StartVisualTransition(float InTargetValue, float InSpeed)
{
	TargetVisualAlpha = InTargetValue;
	TransitionSpeed = InSpeed;
	LastTickTime = -1.0f;
	
	RequestTransitionTick();
}

void UPrismWidgetBase::RequestTransitionTick()
{
	if (UWorld* World = GetWorld())
	{
		if (!World->GetTimerManager().IsTimerActive(TransitionTimer))
		{
			World->GetTimerManager().SetTimer(TransitionTimer, this, &UPrismWidgetBase::InternalTransitionTick, 0.016f, true);
		}
	}
}

void UPrismWidgetBase::OnVisualsUpdated(float InBlendValue)
{
	// Virtual stub for child classes
}

void UPrismWidgetBase::InternalTransitionTick()
{
	UWorld* World = GetWorld();
	if (!World)
		return;

	const float CurrentTime = World->GetTimeSeconds();
	const float ActualDeltaTime = (LastTickTime > 0.0f) ? (CurrentTime - LastTickTime) : 0.016f;
	LastTickTime = CurrentTime;
	
	const bool bKeepTicking = TickTransitions(ActualDeltaTime);
	Invalidate(EInvalidateWidgetReason::Paint);

	if (!bKeepTicking)
	{
		World->GetTimerManager().ClearTimer(TransitionTimer);
		LastTickTime = -1.0f;
	}
}

bool UPrismWidgetBase::TickTransitions(float DeltaTime)
{
	CurrentVisualAlpha = FMath::FInterpTo(CurrentVisualAlpha, TargetVisualAlpha, DeltaTime, TransitionSpeed);
	OnVisualsUpdated(CurrentVisualAlpha);
	
	return !FMath::IsNearlyEqual(CurrentVisualAlpha, TargetVisualAlpha, 0.001f);
}

const FPrismUIWidgetStyle& UPrismWidgetBase::GetDefaultFallbackStyle()
{
	static FPrismUIWidgetStyle Fallback;
	return Fallback;
}

#pragma endregion



#pragma region Getter Setters

UWidgetComponent* UPrismWidgetBase::GetOwningWidgetComponent() const
{
	if (CachedOwningWidgetComponent.IsValid())
		return CachedOwningWidgetComponent.Get();

	UObject* CurrentOuter = GetOuter();

	while (CurrentOuter)
	{
		if (UWidgetComponent* WidgetComp = Cast<UWidgetComponent>(CurrentOuter))
		{
			CachedOwningWidgetComponent = WidgetComp;
			return WidgetComp;
		}

		CurrentOuter = CurrentOuter->GetOuter();
	}

	return nullptr;
}

FVector2D UPrismWidgetBase::GetInteractionMovementDelta(const FPointerEvent& InMouseEvent) const
{
	if (IsRenderedIn3D() && InMouseEvent.GetPointerIndex() != 0)
		return InMouseEvent.GetCursorDelta();

	return InMouseEvent.GetCursorDelta();
}

#pragma endregion