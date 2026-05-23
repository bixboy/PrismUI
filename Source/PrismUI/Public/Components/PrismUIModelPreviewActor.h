#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrismUIModelPreviewActor.generated.h"

class USceneCaptureComponent2D;
class USkeletalMeshComponent;
class UStaticMeshComponent;
class UDirectionalLightComponent;
class USkyLightComponent;
class UTextureRenderTarget2D;


UCLASS(NotBlueprintable)
class PRISMUI_API APrismUIModelPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	APrismUIModelPreviewActor();

	// --- Studio Setup ---

	void SetupForStaticMesh(class UStaticMesh* InMesh);

	void SetupForSkeletalMesh(class USkeletalMesh* InMesh, class UAnimationAsset* InAnimAsset = nullptr, bool bPlayAnim = false);

	void SetCaptureRenderTarget(UTextureRenderTarget2D* InRenderTarget);


	/** Prepares the actor to be returned to the pool (hides meshes, stops capture). */
	void DeactivateStudio();

	/** Rotates the active mesh by a given delta (used for UI dragging). */
	void AddModelRotation(const FRotator& InDeltaRotation);

	/** Sets the absolute rotation of the model. */
	void SetModelRotation(const FRotator& InRotation);

	/** Sets the absolute rotation using a Quaternion (better for interpolation). */
	void SetModelRotationQuat(const FQuat& InQuat);

	FQuat GetModelRotation() const;

	void AutoFrameMesh();
	
	void SetFOV(float InFOV);
	
	void SetCameraOffset(const FVector& InOffset);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<UDirectionalLightComponent> DirectionalLight;
	
	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USkyLightComponent> SkyLight;

	bool bIsAnimated = false;
	
	bool bNeedsOneShotCapture = false;
};
