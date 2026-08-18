#include "Components/PrismUIModelPreviewActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
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

	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(RootComponent);
	SpotLight->SetRelativeLocation(FVector(-200.f, -200.f, 200.f));
	SpotLight->SetRelativeRotation(FRotator(-45.f, 45.f, 0.f));
	SpotLight->Intensity = 5000.f;
	SpotLight->AttenuationRadius = 2000.f;
	
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

// --- Lifecycle ---

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

// --- Studio Setup ---

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
	RequestCapture();
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
		SkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		SkeletalMeshComp->Stop();
		bIsAnimated = false;
		SetActorTickEnabled(false);
		RequestCapture();
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

// --- Render Target Management ---

void APrismUIModelPreviewActor::SetCaptureRenderTarget(UTextureRenderTarget2D* InRenderTarget)
{
	if (SceneCapture)
	{
		SceneCapture->TextureTarget = InRenderTarget;
		RequestCapture();
	}
}

UTextureRenderTarget2D* APrismUIModelPreviewActor::GetOrCreateRenderTarget(int32 InWidth, int32 InHeight)
{
	if (!CachedRenderTarget)
	{
		CachedRenderTarget = NewObject<UTextureRenderTarget2D>(this);
		CachedRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
		CachedRenderTarget->ClearColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.0f);
		CachedRenderTarget->bAutoGenerateMips = false;
	}

	if (CachedRenderTarget->SizeX != InWidth || CachedRenderTarget->SizeY != InHeight)
	{
		CachedRenderTarget->InitAutoFormat(InWidth, InHeight);
		CachedRenderTarget->UpdateResourceImmediate(true);
	}

	SetCaptureRenderTarget(CachedRenderTarget);
	return CachedRenderTarget;
}

// --- Transform & Camera Helpers ---

void APrismUIModelPreviewActor::AddModelRotation(const FRotator& InDeltaRotation)
{
	FQuat YawRot(FVector::UpVector, FMath::DegreesToRadians(InDeltaRotation.Yaw));
	FQuat PitchRot(FVector::RightVector, FMath::DegreesToRadians(InDeltaRotation.Pitch));
	FQuat DeltaQuat = YawRot * PitchRot;

	if (UPrimitiveComponent* ActiveComp = GetActiveMeshComponent())
	{
		ActiveComp->AddWorldRotation(DeltaQuat);
	}
	
	if (!bIsAnimated)
	{
		RequestCapture();
	}
}

void APrismUIModelPreviewActor::SetModelRotation(const FRotator& InRotation)
{
	SetModelRotationQuat(InRotation.Quaternion());
}

void APrismUIModelPreviewActor::SetModelRotationQuat(const FQuat& InQuat)
{
	if (UPrimitiveComponent* ActiveComp = GetActiveMeshComponent())
	{
		ActiveComp->SetWorldRotation(InQuat);
	}

	if (!bIsAnimated)
	{
		RequestCapture();
	}
}

FQuat APrismUIModelPreviewActor::GetModelRotation() const
{
	if (UPrimitiveComponent* ActiveComp = GetActiveMeshComponent())
	{
		return ActiveComp->GetComponentQuat();
	}
	return FQuat::Identity;
}

float APrismUIModelPreviewActor::GetModelBoundsRadius() const
{
	if (UPrimitiveComponent* ActiveComp = GetActiveMeshComponent())
	{
		return ActiveComp->Bounds.SphereRadius;
	}
	return 50.0f;
}

void APrismUIModelPreviewActor::AutoFrameMesh()
{
	FBoxSphereBounds Bounds;
	if (UPrimitiveComponent* ActiveComp = GetActiveMeshComponent())
	{
		Bounds = ActiveComp->Bounds;
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
		
		RequestCapture();
	}
}

void APrismUIModelPreviewActor::SetFOV(float InFOV)
{
	if (SceneCapture)
	{
		SceneCapture->FOVAngle = InFOV;
		RequestCapture();
	}
}

void APrismUIModelPreviewActor::SetCameraOffset(const FVector& InOffset)
{
	if (SceneCapture)
	{
		SceneCapture->SetRelativeLocation(InOffset);
		RequestCapture();
	}
}

FVector APrismUIModelPreviewActor::GetCameraOffset() const
{
	if (SceneCapture)
	{
		return SceneCapture->GetRelativeLocation();
	}
	return FVector::ZeroVector;
}

// --- Internal Helpers ---

void APrismUIModelPreviewActor::RequestCapture()
{
	bNeedsOneShotCapture = true;
	if (SceneCapture && SceneCapture->TextureTarget && !IsActorTickEnabled())
	{
		SceneCapture->CaptureScene();
	}
}

UPrimitiveComponent* APrismUIModelPreviewActor::GetActiveMeshComponent() const
{
	if (StaticMeshComp && StaticMeshComp->IsVisible())
	{
		return StaticMeshComp;
	}
	else if (SkeletalMeshComp && SkeletalMeshComp->IsVisible())
	{
		return SkeletalMeshComp;
	}
	return nullptr;
}
