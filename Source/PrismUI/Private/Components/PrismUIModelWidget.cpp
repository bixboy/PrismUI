#include "Components/PrismUIModelWidget.h"
#include "Components/PrismUIModelPreviewActor.h"
#include "Subsystems/PrismUIModelPreviewSubsystem.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimationAsset.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/Image.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetTree.h"

UPrismUIModelWidget::UPrismUIModelWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	SetIsFocusable(true);

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DefaultMaterial(TEXT("/PrismUI/Material/M_PrismUI_ModelPreview"));
	if (DefaultMaterial.Succeeded())
	{
		BaseMaterial = DefaultMaterial.Object;
	}
}

// --- Lifecycle ---

void UPrismUIModelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	CameraOffset.X = BaseCameraDistance;
	TargetZoomOffset = CameraOffset.X;
	BaseCameraOffset = CameraOffset;

	UWorld* World = GetWorld();
	if (!World)
		return;

	UPrismUIModelPreviewSubsystem* PreviewSubsystem = World->GetSubsystem<UPrismUIModelPreviewSubsystem>();
	if (!PreviewSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UPrismUIModelWidget: Failed to get UPrismUIModelPreviewSubsystem."));
		return;
	}

	int32 ResolutionX = FMath::RoundToInt(PreviewResolution.X);
	int32 ResolutionY = FMath::RoundToInt(PreviewResolution.Y);

	UTextureRenderTarget2D* LocalRT = nullptr;
	PreviewActor = PreviewSubsystem->AcquirePreviewActor(ResolutionX, ResolutionY, LocalRT);
	RenderTarget = LocalRT;

	if (PreviewActor && RenderTarget)
	{
		PreviewActor->SetFOV(FOV);
		UpdatePreviewActor();

		if (BaseMaterial)
		{
			PreviewMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
			if (PreviewMaterial)
			{
				PreviewMaterial->SetTextureParameterValue(FName("RenderTarget"), RenderTarget.Get());
				
				if (ModelImage)
				{
					ModelImage->SetBrushFromMaterial(PreviewMaterial);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("UPrismUIModelWidget: No UImage bound to 'ModelImage'."));
				}
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UPrismUIModelWidget: No BaseMaterial assigned. Preview will not be visible."));
		}
	}
}

void UPrismUIModelWidget::NativeDestruct()
{
	if (PreviewActor)
	{
		if (UWorld* World = GetWorld())
		{
			if (UPrismUIModelPreviewSubsystem* PreviewSubsystem = World->GetSubsystem<UPrismUIModelPreviewSubsystem>())
			{
				PreviewSubsystem->ReleasePreviewActor(PreviewActor, RenderTarget);
			}
		}
	}
	
	PreviewActor = nullptr;
	RenderTarget = nullptr;
	PreviewMaterial = nullptr;

	Super::NativeDestruct();
}

void UPrismUIModelWidget::BuildDefaultLayout()
{
	UWidgetTree* Tree = WidgetTree;
	if (!Tree)
		return;

	if (!ModelImage)
	{
		ModelImage = Tree->ConstructWidget<UImage>(UImage::StaticClass());
		ModelImage->SetVisibility(ESlateVisibility::Visible);
		if (!Tree->RootWidget)
		{
			Tree->RootWidget = ModelImage;
		}
	}
}

// --- API ---

void UPrismUIModelWidget::SetStaticMesh(UStaticMesh* InMesh)
{
	StaticMesh = InMesh;
	SkeletalMesh = nullptr;
	UpdatePreviewActor();
}

void UPrismUIModelWidget::SetSkeletalMesh(USkeletalMesh* InMesh, UAnimationAsset* InAnim, bool bPlay)
{
	SkeletalMesh = InMesh;
	StaticMesh = nullptr;
	AnimationAsset = InAnim;
	bPlayAnimation = bPlay;
	UpdatePreviewActor();
}

// --- Internal Helpers ---

void UPrismUIModelWidget::UpdatePreviewActor()
{
	if (!PreviewActor)
	{
		return;
	}

	if (RenderTarget)
	{
		PreviewActor->SetCaptureRenderTarget(RenderTarget);
	}

	if (SkeletalMesh)
	{
		PreviewActor->SetupForSkeletalMesh(SkeletalMesh, AnimationAsset, bPlayAnimation);
	}
	else if (StaticMesh)
	{
		PreviewActor->SetupForStaticMesh(StaticMesh);
	}
	else
	{
		PreviewActor->DeactivateStudio();
		return;
	}

	PreviewActor->SetModelRotation(BaseRotation);

	if (bAutoFrameModel)
	{
		PreviewActor->AutoFrameMesh();
		
		CameraOffset = PreviewActor->GetCameraOffset();
		BaseCameraDistance = CameraOffset.X;
		TargetZoomOffset = CameraOffset.X;
		BaseCameraOffset = CameraOffset;
		
		ZoomLimits.X = BaseCameraDistance - MaxAutoZoomOutRange;
		
		float ModelRadius = PreviewActor->GetModelBoundsRadius();
		ZoomLimits.Y = -FMath::Max(ModelRadius * 1.2f, 10.0f);
	}
	else
	{
		PreviewActor->SetCameraOffset(CameraOffset);
	}
}

// --- Input Handling ---

FReply UPrismUIModelWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bAllowDragRotation && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		bIsResettingRotation = false;
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	else if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		bIsPanning = true;
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPrismUIModelWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = false;
		if (!bIsPanning) return FReply::Handled().ReleaseMouseCapture();
		return FReply::Handled();
	}
	else if (bIsPanning && InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		bIsPanning = false;
		if (!bIsDragging) return FReply::Handled().ReleaseMouseCapture();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UPrismUIModelWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if ((bIsDragging || bIsPanning) && PreviewActor)
	{
		FVector2D CursorDelta = GetInteractionMovementDelta(InMouseEvent);
		
		if (bIsDragging)
		{
			float YawDelta = CursorDelta.X * DragRotationSpeed * -1.0f;
			float PitchDelta = CursorDelta.Y * DragRotationSpeed * -1.0f;
			CurrentRotationVelocity += FVector2D(YawDelta, PitchDelta);
		}

		if (bIsPanning)
		{
			float ZoomDistance = TargetZoomOffset - BaseCameraDistance;
			if (ZoomDistance > 10.0f) 
			{
				float ZoomFactor = FMath::Clamp(ZoomDistance / FMath::Max(1.0f, FMath::Abs(ZoomLimits.Y - BaseCameraDistance)), 0.01f, 1.0f);
				TargetPanOffset.X -= CursorDelta.X * DragPanSpeed * ZoomFactor;
				TargetPanOffset.Y += CursorDelta.Y * DragPanSpeed * ZoomFactor;
			}
		}

		RequestTransitionTick();
		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UPrismUIModelWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (PreviewActor)
	{
		HandleZoomInput(InMouseEvent.GetWheelDelta());
		RequestTransitionTick();
		return FReply::Handled();
	}
	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

FReply UPrismUIModelWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (PreviewActor && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsResettingRotation = true;
		RequestTransitionTick();
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UPrismUIModelWidget::HandleZoomInput(float ScrollDelta)
{
	if (bAutoFrameModel)
	{
		double CurrentTime = FPlatformTime::Seconds();
		if (CurrentTime - LastZoomTime > 0.4)
		{
			if (FMath::IsNearlyEqual(TargetZoomOffset, BaseCameraDistance, 1.0f))
			{
				ZoomOutResistance = 5.0f;
			}
			else
			{
				ZoomOutResistance = 0.0f;
			}
		}
		LastZoomTime = CurrentTime;

		bool bWasAtOrInsideBase = TargetZoomOffset >= BaseCameraDistance - 0.1f;
		TargetZoomOffset += ScrollDelta * ZoomSpeed;
		
		if (TargetZoomOffset > BaseCameraDistance + 0.1f)
		{
			ZoomOutResistance = 0.0f;
		}

		if (bWasAtOrInsideBase && TargetZoomOffset < BaseCameraDistance && ScrollDelta < 0.0f)
		{
			if (ZoomOutResistance < 5.0f)
			{
				TargetZoomOffset = BaseCameraDistance;
			}
		}
	}
	else
	{
		TargetZoomOffset += ScrollDelta * ZoomSpeed;
	}

	TargetZoomOffset = FMath::Clamp(TargetZoomOffset, ZoomLimits.X, ZoomLimits.Y);
}

// --- Update Logic ---

bool UPrismUIModelWidget::TickTransitions(float DeltaTime)
{
	bool bIsAnimating = Super::TickTransitions(DeltaTime);

	if (!PreviewActor)
	{
		return bIsAnimating;
	}

	UpdateZoom(DeltaTime, bIsAnimating);
	UpdatePanning(DeltaTime, bIsAnimating);
	UpdateRotation(DeltaTime, bIsAnimating);

	return bIsAnimating;
}

void UPrismUIModelWidget::UpdateZoom(float DeltaTime, bool& bOutIsAnimating)
{
	if (!FMath::IsNearlyEqual(CameraOffset.X, TargetZoomOffset, 0.1f))
	{
		CameraOffset.X = FMath::FInterpTo(CameraOffset.X, TargetZoomOffset, DeltaTime, 10.0f);
		PreviewActor->SetCameraOffset(CameraOffset);
		bOutIsAnimating = true;
	}
}

void UPrismUIModelWidget::UpdatePanning(float DeltaTime, bool& bOutIsAnimating)
{
	float ZoomDistance = TargetZoomOffset - BaseCameraDistance;
	float ZoomFactor = FMath::Clamp(ZoomDistance / FMath::Max(1.0f, FMath::Abs(ZoomLimits.Y - BaseCameraDistance)), 0.0f, 1.0f);
	
	float AllowedPanX = MaxPanOffset.X * ZoomFactor;
	float AllowedPanY = MaxPanOffset.Y * ZoomFactor;
	TargetPanOffset.X = FMath::Clamp(TargetPanOffset.X, -AllowedPanX, AllowedPanX);
	TargetPanOffset.Y = FMath::Clamp(TargetPanOffset.Y, -AllowedPanY, AllowedPanY);

	if (!CurrentPanOffset.Equals(TargetPanOffset, 0.1f))
	{
		CurrentPanOffset = FMath::Vector2DInterpTo(CurrentPanOffset, TargetPanOffset, DeltaTime, 10.0f);
		
		CameraOffset.Y = BaseCameraOffset.Y + CurrentPanOffset.X;
		CameraOffset.Z = BaseCameraOffset.Z + CurrentPanOffset.Y;
		
		PreviewActor->SetCameraOffset(CameraOffset);
		bOutIsAnimating = true;
	}
}

void UPrismUIModelWidget::UpdateRotation(float DeltaTime, bool& bOutIsAnimating)
{
	if (!CurrentRotationVelocity.IsNearlyZero(0.1f))
	{
		PreviewActor->AddModelRotation(FRotator(CurrentRotationVelocity.Y, CurrentRotationVelocity.X, 0.0f));
		CurrentRotationVelocity = FMath::Vector2DInterpTo(CurrentRotationVelocity, FVector2D::ZeroVector, DeltaTime, RotationFriction);
		bOutIsAnimating = true;
	}

	if (bIsResettingRotation)
	{
		FQuat CurrentQuat = PreviewActor->GetModelRotation();
		FQuat TargetQuat = BaseRotation.Quaternion();

		if (CurrentQuat.Equals(TargetQuat, 0.001f))
		{
			PreviewActor->SetModelRotationQuat(TargetQuat);
			bIsResettingRotation = false;
		}
		else
		{
			FQuat NewQuat = FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, 12.0f);
			PreviewActor->SetModelRotationQuat(NewQuat);
			bOutIsAnimating = true;
		}
	}
}
