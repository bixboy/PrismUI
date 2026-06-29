#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PrismUIModelPreviewSubsystem.generated.h"

class APrismUIModelPreviewActor;
class UTextureRenderTarget2D;

/**
 * UPrismUIModelPreviewSubsystem — AAA Object Pool for 3D UI Previews.
 * Manages rendering actors to prevent expensive runtime spawning/destruction.
 */
UCLASS()
class PRISMUI_API UPrismUIModelPreviewSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// --- Lifecycle ---
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Pooling Logic ---

	/** 
	 * Acquires a pre-spawned preview actor from the pool, or spawns a new one if necessary. 
	 * Also allocates and returns a transient Render Target.
	 */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Model Preview")
	APrismUIModelPreviewActor* AcquirePreviewActor(int32 InWidth, int32 InHeight, UTextureRenderTarget2D*& OutRenderTarget);

	/** 
	 * Returns the actor to the pool and safely releases the Render Target. 
	 */
	UFUNCTION(BlueprintCallable, Category = "Prism UI | Model Preview")
	void ReleasePreviewActor(APrismUIModelPreviewActor* InActor, UTextureRenderTarget2D* InRenderTarget);

private:
	/** Available actors ready to be used. */
	UPROPERTY()
	TArray<TObjectPtr<APrismUIModelPreviewActor>> AvailableActors;

	/** Actors currently in use by UI widgets. */
	UPROPERTY()
	TArray<TObjectPtr<APrismUIModelPreviewActor>> ActiveActors;

	/** Offset where these actors are spawned to be out of sight. */
	FVector SpawnOffset = FVector(0.0f, 0.0f, -100000.0f);
	
	/** Spacing between pooled actors so they don't capture each other. */
	float StudioSpacing = 5000.0f;
};
