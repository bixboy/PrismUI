#include "Examples/PrismExampleButton.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"

UPrismExampleButton::UPrismExampleButton(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// Set a default role for this specific example button
	WidgetRole = TEXT("Primary");
}

void UPrismExampleButton::NativePreConstruct()
{
	Super::NativePreConstruct();

	// In editor, we ensure the text reflects the default setting
	if (TextBlock)
		TextBlock->SetText(ButtonText);
}

void UPrismExampleButton::NativeConstruct()
{
	Super::NativeConstruct();

	// Initialize visual state
	UpdateTextVisuals(0.0f);
}

void UPrismExampleButton::OnStyleApplied(const FPrismUIWidgetStyle& InStyleData)
{
	Super::OnStyleApplied(InStyleData);

	// Custom logic when a style is applied:
	// For example, matching the background color of the border to the style's core brushes
	if (BackgroundBorder)
		BackgroundBorder->SetBrushColor(InStyleData.CoreBrushes.Normal.TintColor.GetSpecifiedColor());

	// Update text font from the design tokens
	if (TextBlock)
		TextBlock->SetFont(InStyleData.Font);
}

void UPrismExampleButton::OnStateChanged(EPrismWidgetState InNewState)
{
	Super::OnStateChanged(InNewState);

	// Trigger a smooth visual transition using the Prism system
	switch (InNewState)
	{
		case EPrismWidgetState::Hovered:
			StartVisualTransition(1.0f, 20.0f); // Fast transition to hovered state
			break;

		case EPrismWidgetState::Normal:
		default:
			StartVisualTransition(0.0f, 10.0f); // Slower return to normal
			break;
	}

	// Logging interaction events as per AAA standards
	UE_LOG(LogTemp, Log, TEXT("Button [%s] State Changed to: %d"), *GetName(), (int32)InNewState);
}

void UPrismExampleButton::OnVisualsUpdated(float InBlendValue)
{
	Super::OnVisualsUpdated(InBlendValue);

	// This function is called every frame during a transition
	UpdateTextVisuals(InBlendValue);
}

void UPrismExampleButton::UpdateTextVisuals(float InAlpha)
{
	if (!TextBlock)
		return;

	// Interpolate between normal and hovered opacity
	const float FinalOpacity = FMath::Lerp(1.0f, HoverTextOpacity, InAlpha);
	TextBlock->SetRenderOpacity(FinalOpacity);

	// Add a slight scale effect for "premium" feel
	const float Scale = FMath::Lerp(1.0f, 1.05f, InAlpha);
	TextBlock->SetRenderScale(FVector2D(Scale, Scale));
}
