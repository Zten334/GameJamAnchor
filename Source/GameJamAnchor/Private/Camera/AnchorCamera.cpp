// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/AnchorCamera.h"
#include "Camera/CameraComponent.h"

AAnchorCamera::AAnchorCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	GetCameraComponent()->SetProjectionMode(ECameraProjectionMode::Orthographic);
	UpdateOrthographicSettings();
}

void AAnchorCamera::UpdateOrthographicSettings()
{
	if (UCameraComponent* CameraComp = GetCameraComponent())
	{
		CameraComp->SetOrthoWidth(ViewportWidth);
		CameraComp->SetAspectRatio(ViewportWidth / ViewportHeight);
		CameraComp->bConstrainAspectRatio = true;

		// UE5 默认的自动曝光/色调曲线/泛光会让未照明的 Paper2D Sprite 在 PIE 里过曝发白。
		// 2D 像素项目直接固定为手动曝光并关闭相关后处理，保证颜色与编辑器一致。
		CameraComp->PostProcessBlendWeight = 1.0f;

		CameraComp->PostProcessSettings.bOverride_AutoExposureMethod = true;
		CameraComp->PostProcessSettings.AutoExposureMethod = AEM_Manual;

		CameraComp->PostProcessSettings.bOverride_AutoExposureApplyPhysicalCameraExposure = true;
		CameraComp->PostProcessSettings.AutoExposureApplyPhysicalCameraExposure = false;

		CameraComp->PostProcessSettings.bOverride_AutoExposureBias = true;
		CameraComp->PostProcessSettings.AutoExposureBias = 0.0f;

		CameraComp->PostProcessSettings.bOverride_ToneCurveAmount = true;
		CameraComp->PostProcessSettings.ToneCurveAmount = 0.0f;

		CameraComp->PostProcessSettings.bOverride_BloomIntensity = true;
		CameraComp->PostProcessSettings.BloomIntensity = 0.0f;

		CameraComp->PostProcessSettings.bOverride_MotionBlurAmount = true;
		CameraComp->PostProcessSettings.MotionBlurAmount = 0.0f;

		CameraComp->PostProcessSettings.bOverride_MotionBlurMax = true;
		CameraComp->PostProcessSettings.MotionBlurMax = 0.0f;
	}
}

#if WITH_EDITOR
void AAnchorCamera::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateOrthographicSettings();
}
#endif
