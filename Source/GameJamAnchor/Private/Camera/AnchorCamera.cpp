// Copyright Epic Games, Inc. All Rights Reserved.

#include "Camera/AnchorCamera.h"
#include "Camera/CameraComponent.h"

AAAnchorCamera::AAAnchorCamera(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	GetCameraComponent()->SetProjectionMode(ECameraProjectionMode::Orthographic);
	UpdateOrthographicSettings();
}

void AAAnchorCamera::UpdateOrthographicSettings()
{
	if (UCameraComponent* CameraComp = GetCameraComponent())
	{
		CameraComp->SetOrthoWidth(ViewportWidth);
		CameraComp->SetAspectRatio(ViewportWidth / ViewportHeight);
		CameraComp->bConstrainAspectRatio = true;
	}
}

#if WITH_EDITOR
void AAAnchorCamera::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateOrthographicSettings();
}
#endif
