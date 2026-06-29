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

FLinearColor UPrismUIThemeSubsystem::GetColorToken(FName InRole, EPrismColorToken InToken) const
{
	if (UPrismUIStyle* Style = GetStyleForRole(InRole))
	{
		if (const FLinearColor* ColorPtr = Style->StyleData.Colors.Find(InToken))
		{
			return *ColorPtr;
		}
	}
	return FLinearColor::White;
}

FSlateFontInfo UPrismUIThemeSubsystem::GetTypographyToken(FName InRole, EPrismTypographyToken InToken) const
{
	if (UPrismUIStyle* Style = GetStyleForRole(InRole))
	{
		if (const FSlateFontInfo* FontPtr = Style->StyleData.Typography.Find(InToken))
		{
			return *FontPtr;
		}
	}
	return FSlateFontInfo();
}
