#pragma once
#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismSimulatorTest.generated.h"

class UVerticalBox;
class UPrismButtonBase;

UCLASS()
class PRISMUI_API UPrismSimulatorTest : public UPrismWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	void BuildTestLayout();

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> MainContainer;

	UPROPERTY(EditAnywhere, Category = "Prism UI")
	TSubclassOf<UPrismButtonBase> ButtonClass;
};
