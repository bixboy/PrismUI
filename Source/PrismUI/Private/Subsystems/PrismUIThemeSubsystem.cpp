#include "Subsystems/PrismUIThemeSubsystem.h"
#include "Styles/PrismUIStyle.h"


void UPrismUIThemeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPrismUIThemeSubsystem::Deinitialize()
{
	ActiveTheme = nullptr;
	Super::Deinitialize();
}

void UPrismUIThemeSubsystem::SetActiveTheme(UPrismUIThemeData* InTheme)
{
	if (ActiveTheme == InTheme)
		return;

	ActiveTheme = InTheme;
	OnThemeChanged.Broadcast(ActiveTheme);
}

UPrismUIStyle* UPrismUIThemeSubsystem::GetStyleForRole(FName InRole) const
{
	if (!ActiveTheme)
		return nullptr;

	return ActiveTheme->GetStyleForRole(InRole);
}
