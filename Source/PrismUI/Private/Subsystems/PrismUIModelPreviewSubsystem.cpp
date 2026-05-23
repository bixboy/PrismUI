#include "Subsystems/PrismUIModelPreviewSubsystem.h"
#include "Components/PrismUIModelPreviewActor.h"
#include "Engine/World.h"
#include "Engine/TextureRenderTarget2D.h"

bool UPrismUIModelPreviewSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	if (World)
		return World->IsGameWorld();

	return false;
}

void UPrismUIModelPreviewSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UPrismUIModelPreviewSubsystem::Deinitialize()
{
	for (APrismUIModelPreviewActor* Actor : AvailableActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	AvailableActors.Empty();

	for (APrismUIModelPreviewActor* Actor : ActiveActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	ActiveActors.Empty();

	Super::Deinitialize();
}

APrismUIModelPreviewActor* UPrismUIModelPreviewSubsystem::AcquirePreviewActor(int32 InWidth, int32 InHeight, UTextureRenderTarget2D*& OutRenderTarget)
{
	UWorld* World = GetWorld();
	if (!World)
		return nullptr;


	APrismUIModelPreviewActor* AcquiredActor = nullptr;

	// 1. Try to get from pool
	if (AvailableActors.Num() > 0)
	{
		AcquiredActor = AvailableActors.Pop(EAllowShrinking::No);
	}
	else
	{
		// 2. Spawn a new one if pool is empty
		int32 StudioIndex = ActiveActors.Num();
		FVector SpawnLocation = SpawnOffset + FVector(StudioIndex * StudioSpacing, 0.0f, 0.0f);
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;

		AcquiredActor = World->SpawnActor<APrismUIModelPreviewActor>(APrismUIModelPreviewActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	}

	if (AcquiredActor)
	{
		ActiveActors.Add(AcquiredActor);

		OutRenderTarget = NewObject<UTextureRenderTarget2D>(this);
		check(OutRenderTarget);
		OutRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		OutRenderTarget->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
		OutRenderTarget->bAutoGenerateMips = false;
		OutRenderTarget->InitAutoFormat(InWidth, InHeight);
		OutRenderTarget->UpdateResourceImmediate(true);

		AcquiredActor->SetCaptureRenderTarget(OutRenderTarget);
	}

	return AcquiredActor;
}

void UPrismUIModelPreviewSubsystem::ReleasePreviewActor(APrismUIModelPreviewActor* InActor, UTextureRenderTarget2D* InRenderTarget)
{
	if (!InActor)
		return;

	InActor->DeactivateStudio();

	if (ActiveActors.Remove(InActor) > 0)
	{
		AvailableActors.Add(InActor);
	}

}
