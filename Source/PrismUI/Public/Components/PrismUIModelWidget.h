// Copyright (c) Bixboy, 2026. All Rights Reserved.
#pragma once
#include "CoreMinimal.h"
#include "PrismWidgetBase.h"
#include "PrismUIModelWidget.generated.h"

class UStaticMesh;
class USkeletalMesh;
class UAnimationAsset;
class APrismUIModelPreviewActor;
class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UImage;


UCLASS()
class PRISMUI_API UPrismUIModelWidget : public UPrismWidgetBase
{
	GENERATED_BODY()

public:
	UPrismUIModelWidget(const FObjectInitializer& ObjectInitializer);

	// --- Configuration ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	TObjectPtr<UMaterialInterface> BaseMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	TObjectPtr<UStaticMesh> StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	TObjectPtr<USkeletalMesh> SkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	TObjectPtr<UAnimationAsset> AnimationAsset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	bool bPlayAnimation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	float FOV = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	FVector CameraOffset = FVector(-300.0f, 0.0f, 50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	bool bAutoFrameModel = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	FRotator BaseRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	bool bAllowDragRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	float DragRotationSpeed = 1.0f;

	// --- API ---

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Model Preview")
	void SetStaticMesh(UStaticMesh* InMesh);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Model Preview")
	void SetSkeletalMesh(USkeletalMesh* InMesh, UAnimationAsset* InAnim = nullptr, bool bPlay = true);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void BuildDefaultLayout() override;
	
	// --- Input Handling for Rotation ---
	virtual bool TickTransitions(float DeltaTime) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ModelImage;

private:
	void UpdatePreviewActor();

	UPROPERTY()
	TObjectPtr<APrismUIModelPreviewActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;

	bool bIsDragging = false;

	bool bIsResettingRotation = false;
};
