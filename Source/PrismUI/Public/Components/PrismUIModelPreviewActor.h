#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrismUIModelPreviewActor.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class USpotLightComponent;
class USkyLightComponent;
class UAnimationAsset;

UCLASS(NotBlueprintable)
class PRISMUI_API APrismUIModelPreviewActor : public AActor
{
	GENERATED_BODY()

public:
	APrismUIModelPreviewActor();

	// --- Lifecycle ---
	virtual void Tick(float DeltaTime) override;

	// --- Studio Setup ---
	void SetupForStaticMesh(UStaticMesh* InMesh);
	void SetupForSkeletalMesh(USkeletalMesh* InMesh, UAnimationAsset* InAnimAsset = nullptr, bool bPlayAnim = false);
	void DeactivateStudio();

	// --- Render Target Management ---
	void SetCaptureRenderTarget(UTextureRenderTarget2D* InRenderTarget);
	/** Gets or creates the cached render target with the specified dimensions. */
	UTextureRenderTarget2D* GetOrCreateRenderTarget(int32 InWidth, int32 InHeight);

	// --- Transform & Camera Helpers ---
	void AddModelRotation(const FRotator& InDeltaRotation);
	void SetModelRotation(const FRotator& InRotation);
	/** Sets the absolute rotation using a Quaternion (better for interpolation). */
	void SetModelRotationQuat(const FQuat& InQuat);
	FQuat GetModelRotation() const;
	
	float GetModelBoundsRadius() const;

	void AutoFrameMesh();
	void SetFOV(float InFOV);
	
	void SetCameraOffset(const FVector& InOffset);
	FVector GetCameraOffset() const;

protected:
	// --- Lifecycle ---
	virtual void BeginPlay() override;

	// --- Internal Helpers ---
	void RequestCapture();
	UPrimitiveComponent* GetActiveMeshComponent() const;

private:
	// --- Components ---
	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USpotLightComponent> SpotLight;
	
	UPROPERTY(VisibleAnywhere, Category = "Prism UI | Preview")
	TObjectPtr<USkyLightComponent> SkyLight;

	// --- State ---
	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CachedRenderTarget;

	bool bIsAnimated = false;
	bool bNeedsOneShotCapture = false;
};
