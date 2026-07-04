// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "AnchorCamera.generated.h"

/**
 * 静态正交相机，负责框定 180×320 的 2D 可视区域。
 * 不跟随玩家，由 GameMode 在玩家加入后设置为 ViewTarget。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API AAAnchorCamera : public ACameraActor
{
	GENERATED_BODY()

public:
	AAAnchorCamera(const FObjectInitializer& ObjectInitializer);

protected:
	/** 逻辑视口宽度（世界单位）。默认 180，对应 180×320 像素画风的宽。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Camera", meta = (ToolTip = "逻辑视口宽度（世界单位）。默认 180，对应 180×320 像素画风的宽。"))
	float ViewportWidth = 180.0f;

	/** 逻辑视口高度（世界单位）。默认 320，对应 180×320 像素画风的高。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Camera", meta = (ToolTip = "逻辑视口高度（世界单位）。默认 320，对应 180×320 像素画风的高。"))
	float ViewportHeight = 320.0f;

	/** 根据 ViewportWidth / ViewportHeight 设置正交相机的 OrthoWidth 与 AspectRatio。 */
	void UpdateOrthographicSettings();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
