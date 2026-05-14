#include "PrismWidgetBase.h"
#include "Subsystems/PrismUIThemeSubsystem.h"
#include "Animation/WidgetAnimation.h"


UPrismWidgetBase::UPrismWidgetBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bIsAttributesDirty = true;
}

// --- Lifecycle ---

void UPrismWidgetBase::NativePreConstruct()
{
	Super::NativePreConstruct();
	BuildDefaultLayout();
	RefreshStyle();
}

void UPrismWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildDefaultLayout();
}

void UPrismWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	if (UPrismUIThemeSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UPrismUIThemeSubsystem>())
	{
		if (!Subsystem->OnThemeChanged.IsAlreadyBound(this, &UPrismWidgetBase::HandleThemeChanged))
			Subsystem->OnThemeChanged.AddUniqueDynamic(this, &UPrismWidgetBase::HandleThemeChanged);
	}

	RefreshStyle();
	OnVisualsUpdated(CurrentVisualAlpha);
}

void UPrismWidgetBase::NativeDestruct()
{
	if (UPrismUIThemeSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UPrismUIThemeSubsystem>())
		Subsystem->OnThemeChanged.RemoveAll(this);

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

void UPrismWidgetBase::RefreshStyle()
{
	OnStyleApplied(GetEffectiveStyle());
	
}

const FPrismUIWidgetStyle& UPrismWidgetBase::GetEffectiveStyle() const
{
	if (StyleOverride)
		return StyleOverride->StyleData;

	if (UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			if (UPrismUIThemeSubsystem* Subsystem = GI->GetSubsystem<UPrismUIThemeSubsystem>())
			{
				if (UPrismUIStyle* Style = Subsystem->GetStyleForRole(WidgetRole))
					return Style->StyleData;
			}
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
	
	// Reset tick time to ensure the first delta is calculated correctly
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
	
	// Return true if we are still far from target
	return !FMath::IsNearlyEqual(CurrentVisualAlpha, TargetVisualAlpha, 0.001f);
}

const FPrismUIWidgetStyle& UPrismWidgetBase::GetDefaultFallbackStyle()
{
	static FPrismUIWidgetStyle Fallback;
	return Fallback;
}

#pragma endregion