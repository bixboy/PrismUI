#include "Components/PrismProgressBarBase.h"


void UPrismProgressBarBase::SetPercent(float InPercent)
{
	const float NewPercent = FMath::Clamp(InPercent, 0.0f, 1.0f);
	if (FMath::IsNearlyEqual(CurrentPercent, NewPercent))
		return;

	CurrentPercent = NewPercent;
	RefreshStyle();
}
