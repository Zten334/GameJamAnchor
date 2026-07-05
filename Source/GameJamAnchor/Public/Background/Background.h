// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Background.generated.h"

/**
 * 背景 Actor。
 * 可通过 BackgroundSprite 直接指定 PaperSprite，或通过 BackgroundTexture 指定源纹理运行时生成 Sprite。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API ABackground : public AActor
{
	GENERATED_BODY()

public:
	ABackground(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Background", meta = (ToolTip = "根场景组件。"))
	TObjectPtr<class USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Background", meta = (ToolTip = "背景的 Sprite 显示组件。"))
	TObjectPtr<class UPaperSpriteComponent> SpriteComponent;

	/** 背景图 PaperSprite 资产。优先级最高，设置后直接使用。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Background", meta = (ToolTip = "直接指定一张 PaperSprite 作为背景。设置后将优先使用此项。"))
	TObjectPtr<class UPaperSprite> BackgroundSprite;

	/** 背景图源纹理。如果没有指定 BackgroundSprite，会基于该纹理在运行时生成一个 PaperSprite。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Background", meta = (ToolTip = "指定一张 Texture2D 作为背景图源。如果没有设置 BackgroundSprite，会基于该纹理运行时生成 PaperSprite。"))
	TObjectPtr<class UTexture2D> BackgroundTexture;

	/** 背景在世界空间中的宽度（默认 180）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Background", meta = (ToolTip = "背景在世界空间中的宽度，默认 180，与逻辑视口宽度一致。"))
	float BackgroundWidth = 180.0f;

	/** 背景在世界空间中的高度（默认 320）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Background", meta = (ToolTip = "背景在世界空间中的高度，默认 320，与逻辑视口高度一致。"))
	float BackgroundHeight = 320.0f;

	/** 背景色调。默认白色（不染色）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Background", meta = (ToolTip = "背景色调，默认白色（不染色）。如果想给背景图染色，可调整此颜色。"))
	FLinearColor BackgroundColor = FLinearColor::White;

	class UPaperSprite* CreateSpriteFromTexture(UTexture2D* Texture);
};
