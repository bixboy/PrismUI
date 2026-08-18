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
	bool bAutoFrameModel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	FRotator BaseRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	bool bAllowDragRotation = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	float DragRotationSpeed = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	float RotationFriction = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	FVector2D PreviewResolution = FVector2D(512.0f, 512.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	float ZoomSpeed = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	FVector2D ZoomLimits = FVector2D(-1000.0f, -50.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview", meta = (EditCondition = "bAutoFrameModel"))
	float MaxAutoZoomOutRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	float BaseCameraDistance = -300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	float DragPanSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prism UI | Model Preview")
	FVector2D MaxPanOffset = FVector2D(150.0f, 150.0f);

	// --- API ---

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Model Preview")
	void SetStaticMesh(UStaticMesh* InMesh);

	UFUNCTION(BlueprintCallable, Category = "Prism UI | Model Preview")
	void SetSkeletalMesh(USkeletalMesh* InMesh, UAnimationAsset* InAnim = nullptr, bool bPlay = true);

protected:
	// --- Lifecycle ---
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void BuildDefaultLayout() override;
	virtual bool TickTransitions(float DeltaTime) override;
	
	// --- Input Handling ---
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ModelImage;

private:
	// --- Internal Helpers ---
	void UpdatePreviewActor();
	void HandleZoomInput(float ScrollDelta);
	
	// --- Update Logic ---
	void UpdateZoom(float DeltaTime, bool& bOutIsAnimating);
	void UpdatePanning(float DeltaTime, bool& bOutIsAnimating);
	void UpdateRotation(float DeltaTime, bool& bOutIsAnimating);

	// --- Component References ---
	UPROPERTY()
	TObjectPtr<APrismUIModelPreviewActor> PreviewActor;

	UPROPERTY()
	TObjectPtr<UTextureRenderTarget2D> RenderTarget;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;

	// --- State variables ---
	bool bIsDragging = false;
	bool bIsPanning = false;
	bool bIsResettingRotation = false;

	FVector2D CurrentRotationVelocity = FVector2D::ZeroVector;

	float ZoomOutResistance = 0.0f;
	double LastZoomTime = 0.0;

	float TargetZoomOffset = 0.0f;
	
	FVector2D TargetPanOffset = FVector2D::ZeroVector;
	FVector2D CurrentPanOffset = FVector2D::ZeroVector;
	FVector BaseCameraOffset = FVector::ZeroVector;
};
