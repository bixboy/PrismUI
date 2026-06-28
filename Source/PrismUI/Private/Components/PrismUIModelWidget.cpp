// Copyright (c) Bixboy, 2026. All Rights Reserved.
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

void UPrismUIModelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UWorld* World = GetWorld();
	if (!World)
		return;

	UPrismUIModelPreviewSubsystem* PreviewSubsystem = World->GetSubsystem<UPrismUIModelPreviewSubsystem>();
	if (!PreviewSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("UPrismUIModelWidget: Failed to get UPrismUIModelPreviewSubsystem."));
		return;
	}

	int32 ResolutionX = 512;
	int32 ResolutionY = 512;

	UTextureRenderTarget2D* LocalRT = nullptr;
	PreviewActor = PreviewSubsystem->AcquirePreviewActor(ResolutionX, ResolutionY, LocalRT);
	RenderTarget = LocalRT;

	if (PreviewActor && RenderTarget)
	{
		// Configure Actor Camera
		PreviewActor->SetFOV(FOV);

		// Apply Model
		UpdatePreviewActor();

		// Setup Material
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
					UE_LOG(LogTemp, Warning, TEXT("UPrismUIModelWidget: No UImage bound to 'ModelImage'. Please ensure your UMG widget has an Image named ModelImage."));
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
		UWorld* World = GetWorld();
		if (World)
		{
			UPrismUIModelPreviewSubsystem* PreviewSubsystem = World->GetSubsystem<UPrismUIModelPreviewSubsystem>();
			if (PreviewSubsystem)
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

void UPrismUIModelWidget::UpdatePreviewActor()
{
	if (!PreviewActor)
	{
		return;
	}

	if (SkeletalMesh)
	{
		if (RenderTarget)
		{
			PreviewActor->SetCaptureRenderTarget(RenderTarget);
		}
		PreviewActor->SetupForSkeletalMesh(SkeletalMesh, AnimationAsset, bPlayAnimation);
	}
	else if (StaticMesh)
	{
		if (RenderTarget)
		{
			PreviewActor->SetCaptureRenderTarget(RenderTarget);
		}
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
	}
	else
	{
		PreviewActor->SetCameraOffset(CameraOffset);
	}
}

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

FReply UPrismUIModelWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bAllowDragRotation && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = true;
		bIsResettingRotation = false;
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UPrismUIModelWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UPrismUIModelWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDragging && PreviewActor)
	{
		FVector2D CursorDelta = GetInteractionMovementDelta(InMouseEvent);
		
		float YawDelta = CursorDelta.X * DragRotationSpeed * -1.0f;
		float PitchDelta = CursorDelta.Y * DragRotationSpeed * -1.0f;

		PreviewActor->AddModelRotation(FRotator(PitchDelta, YawDelta, 0.0f));

		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
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

bool UPrismUIModelWidget::TickTransitions(float DeltaTime)
{
	bool bIsAnimating = Super::TickTransitions(DeltaTime);

	if (bIsResettingRotation && PreviewActor)
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
			bIsAnimating = true;
		}
	}

	return bIsAnimating;
}
