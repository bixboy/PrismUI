// Copyright (c) Bixboy, 2026. All Rights Reserved.
#include "Components/PrismUIModelPreviewActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"


APrismUIModelPreviewActor::APrismUIModelPreviewActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	SceneCapture->SetupAttachment(RootComponent);
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;

	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
	DirectionalLight->SetupAttachment(RootComponent);
	DirectionalLight->SetRelativeRotation(FRotator(-45.f, 45.f, 0.f));
	DirectionalLight->Intensity = 3.0f;
	
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->Intensity = 1.0f;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);
	StaticMeshComp->SetVisibility(false);
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(RootComponent);
	SkeletalMeshComp->SetVisibility(false);
	SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SceneCapture->ShowOnlyActors.Add(this);
}

void APrismUIModelPreviewActor::BeginPlay()
{
	Super::BeginPlay();
}

void APrismUIModelPreviewActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsAnimated || bNeedsOneShotCapture)
	{
		if (SceneCapture && SceneCapture->TextureTarget)
		{
			SceneCapture->CaptureScene();
		}
		bNeedsOneShotCapture = false;
	}
}

void APrismUIModelPreviewActor::SetupForStaticMesh(UStaticMesh* InMesh)
{
	if (!InMesh)
	{
		DeactivateStudio();
		return;
	}

	SkeletalMeshComp->SetVisibility(false);
	SkeletalMeshComp->SetSkeletalMesh(nullptr);

	StaticMeshComp->SetStaticMesh(InMesh);
	StaticMeshComp->SetVisibility(true);
	
	StaticMeshComp->SetRelativeTransform(FTransform::Identity);

	bIsAnimated = false;
	SetActorTickEnabled(false);
	bNeedsOneShotCapture = true;
	
	if (SceneCapture && SceneCapture->TextureTarget)
	{
		SceneCapture->CaptureScene();
	}
}

void APrismUIModelPreviewActor::SetupForSkeletalMesh(USkeletalMesh* InMesh, UAnimationAsset* InAnimAsset, bool bPlayAnim)
{
	if (!InMesh)
	{
		DeactivateStudio();
		return;
	}

	StaticMeshComp->SetVisibility(false);
	StaticMeshComp->SetStaticMesh(nullptr);

	SkeletalMeshComp->SetSkeletalMesh(InMesh);
	SkeletalMeshComp->SetVisibility(true);
	
	SkeletalMeshComp->SetRelativeTransform(FTransform::Identity);

	if (InAnimAsset && bPlayAnim)
	{
		SkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SkeletalMeshComp->PlayAnimation(InAnimAsset, true);
		bIsAnimated = true;
		SetActorTickEnabled(true);
	}
	else
	{
		SkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint); // Or keep default
		SkeletalMeshComp->Stop();
		bIsAnimated = false;
		SetActorTickEnabled(false);
		bNeedsOneShotCapture = true;
		
		if (SceneCapture && SceneCapture->TextureTarget)
		{
			SceneCapture->CaptureScene();
		}
	}
}

void APrismUIModelPreviewActor::SetCaptureRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	if (SceneCapture)
	{
		SceneCapture->TextureTarget = InRenderTarget;
		bNeedsOneShotCapture = true;
	}
}

void APrismUIModelPreviewActor::DeactivateStudio()
{
	bIsAnimated = false;
	SetActorTickEnabled(false);
	
	if (SceneCapture)
	{
		SceneCapture->TextureTarget = nullptr;
	}
	
	if (StaticMeshComp)
	{
		StaticMeshComp->SetVisibility(false);
		StaticMeshComp->SetStaticMesh(nullptr);
	}

	if (SkeletalMeshComp)
	{
		SkeletalMeshComp->SetVisibility(false);
		SkeletalMeshComp->SetSkeletalMesh(nullptr);
		SkeletalMeshComp->Stop();
	}
}

void APrismUIModelPreviewActor::AddModelRotation(const FRotator& InDeltaRotation)
{
	FQuat YawRot(FVector::UpVector, FMath::DegreesToRadians(InDeltaRotation.Yaw));
	FQuat PitchRot(FVector::RightVector, FMath::DegreesToRadians(InDeltaRotation.Pitch));
	FQuat DeltaQuat = YawRot * PitchRot;

	if (StaticMeshComp->IsVisible())
	{
		StaticMeshComp->AddWorldRotation(DeltaQuat);
	}
	else if (SkeletalMeshComp->IsVisible())
	{
		SkeletalMeshComp->AddWorldRotation(DeltaQuat);
	}
	
	if (!bIsAnimated)
	{
		bNeedsOneShotCapture = true;
		if (SceneCapture && SceneCapture->TextureTarget && !IsActorTickEnabled())
		{
			SceneCapture->CaptureScene();
		}
	}
}

void APrismUIModelPreviewActor::SetModelRotation(const FRotator& InRotation)
{
	SetModelRotationQuat(InRotation.Quaternion());
}

void APrismUIModelPreviewActor::SetModelRotationQuat(const FQuat& InQuat)
{
	if (StaticMeshComp->IsVisible())
	{
		StaticMeshComp->SetWorldRotation(InQuat);
	}
	else if (SkeletalMeshComp->IsVisible())
	{
		SkeletalMeshComp->SetWorldRotation(InQuat);
	}

	if (!bIsAnimated)
	{
		bNeedsOneShotCapture = true;
		if (SceneCapture && SceneCapture->TextureTarget && !IsActorTickEnabled())
		{
			SceneCapture->CaptureScene();
		}
	}
}

FQuat APrismUIModelPreviewActor::GetModelRotation() const
{
	if (StaticMeshComp->IsVisible())
	{
		return StaticMeshComp->GetComponentQuat();
	}
	else if (SkeletalMeshComp->IsVisible())
	{
		return SkeletalMeshComp->GetComponentQuat();
	}
	return FQuat::Identity;
}

void APrismUIModelPreviewActor::AutoFrameMesh()
{
	FBoxSphereBounds Bounds;
	if (StaticMeshComp->IsVisible() && StaticMeshComp->GetStaticMesh())
	{
		Bounds = StaticMeshComp->Bounds;
	}
	else if (SkeletalMeshComp->IsVisible() && SkeletalMeshComp->GetSkeletalMeshAsset())
	{
		Bounds = SkeletalMeshComp->Bounds;
	}
	else
	{
		return;
	}

	if (SceneCapture)
	{
		float HalfFOV = FMath::DegreesToRadians(SceneCapture->FOVAngle * 0.5f);
		float Distance = (Bounds.SphereRadius * 1.15f) / FMath::Sin(HalfFOV);

		FVector LocalOrigin = GetTransform().InverseTransformPosition(Bounds.Origin);

		FVector NewCameraLocation = LocalOrigin - FVector(Distance, 0.0f, 0.0f);
		SceneCapture->SetRelativeLocation(NewCameraLocation);
		
		bNeedsOneShotCapture = true;
		if (SceneCapture->TextureTarget && !IsActorTickEnabled())
		{
			SceneCapture->CaptureScene();
		}
	}
}

void APrismUIModelPreviewActor::SetFOV(float InFOV)
{
	if (SceneCapture)
	{
		SceneCapture->FOVAngle = InFOV;
		bNeedsOneShotCapture = true;
	}
}

void APrismUIModelPreviewActor::SetCameraOffset(const FVector& InOffset)
{
	if (SceneCapture)
	{
		SceneCapture->SetRelativeLocation(InOffset);
		bNeedsOneShotCapture = true;
	}
}
