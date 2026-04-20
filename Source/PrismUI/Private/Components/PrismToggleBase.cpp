#include "Components/PrismToggleBase.h"


void UPrismToggleBase::NativeDestruct()
{
	Super::NativeDestruct();
}

void UPrismToggleBase::SetIsChecked(bool bInIsChecked, bool bInBroadcast)
{
	if (bIsChecked == bInIsChecked)
		return;

	bIsChecked = bInIsChecked;
    TargetToggleAlpha = bIsChecked ? 1.0f : 0.0f;

    // Wake up the async transition engine
    RequestTransitionTick();

	if (bInBroadcast)
		OnToggleChanged.Broadcast(this, bIsChecked);
}

bool UPrismToggleBase::TickTransitions(float DeltaTime)
{
    // 1. Process standard Widget-level transition (Hover/Press)
    bool bBaseTicking = Super::TickTransitions(DeltaTime);

    // 2. Process Toggle-Specific Animation
    ToggleAlpha = FMath::FInterpTo(ToggleAlpha, TargetToggleAlpha, DeltaTime, 15.0f);

    bool bToggleTicking = !FMath::IsNearlyEqual(ToggleAlpha, TargetToggleAlpha, 0.001f);

    // 3. Keep ticking if either base or toggle is still animating
    return bBaseTicking || bToggleTicking;
}

void UPrismToggleBase::HandleClick()
{
	SetIsChecked(!bIsChecked);
	Super::HandleClick();
}
