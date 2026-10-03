#include "Components/PrismUIModelPreviewActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMathLibrary.h"

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
	SceneCapture->ShowFlags.SetEyeAdaptation(false);
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;

	KeyLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(RootComponent);
	KeyLight->SetRelativeLocation(FVector(-250.f, -160.f, 200.f));
	KeyLight->SetRelativeRotation(FRotator(-22.f, 32.f, 0.f));
	KeyLight->Intensity = 6000.f;
	KeyLight->AttenuationRadius = 4000.f;
	KeyLight->InnerConeAngle = 45.0f;
	KeyLight->OuterConeAngle = 70.0f;
	KeyLight->LightColor = FColor(255, 252, 248);
	KeyLight->LightingChannels.bChannel0 = false;
	KeyLight->LightingChannels.bChannel1 = true;
	
	FillLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(RootComponent);
	FillLight->SetRelativeLocation(FVector(-220.f, 160.f, 160.f));
	FillLight->SetRelativeRotation(FRotator(-18.f, -36.f, 0.f));
	FillLight->Intensity = 6000.f;
	FillLight->AttenuationRadius = 4000.f;
	FillLight->InnerConeAngle = 55.0f;
	FillLight->OuterConeAngle = 80.0f;
	FillLight->LightColor = FColor(245, 248, 255);
	FillLight->LightingChannels.bChannel0 = false;
	FillLight->LightingChannels.bChannel1 = true;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);
	StaticMeshComp->SetVisibility(false);
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StaticMeshComp->LightingChannels.bChannel0 = false;
	StaticMeshComp->LightingChannels.bChannel1 = true;
	
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(RootComponent);
	SkeletalMeshComp->SetVisibility(false);
	SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SkeletalMeshComp->LightingChannels.bChannel0 = false;
	SkeletalMeshComp->LightingChannels.bChannel1 = true;

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
	AutoFrameMesh();
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
	AutoFrameMesh();

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
		
		UpdateStudioLights(LocalOrigin, Bounds.SphereRadius);

		RequestCapture();
	}
}

void APrismUIModelPreviewActor::UpdateStudioLights(const FVector& LocalOrigin, float SphereRadius)
{
	float SafeRadius = FMath::Max(SphereRadius, 40.0f);
	float Attenuation = FMath::Max(SafeRadius * 5.0f, 2500.0f);

	if (KeyLight)
	{
		FVector KeyLoc = LocalOrigin + FVector(-SafeRadius * 2.2f, -SafeRadius * 1.5f, SafeRadius * 1.1f);
		KeyLight->SetRelativeLocation(KeyLoc);
		KeyLight->SetRelativeRotation(UKismetMathLibrary::FindLookAtRotation(KeyLoc, LocalOrigin));
		KeyLight->AttenuationRadius = Attenuation;
	}

	if (FillLight)
	{
		FVector FillLoc = LocalOrigin + FVector(-SafeRadius * 2.0f, SafeRadius * 1.5f, SafeRadius * 0.5f);
		FillLight->SetRelativeLocation(FillLoc);
		FillLight->SetRelativeRotation(UKismetMathLibrary::FindLookAtRotation(FillLoc, LocalOrigin));
		FillLight->AttenuationRadius = Attenuation;
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
