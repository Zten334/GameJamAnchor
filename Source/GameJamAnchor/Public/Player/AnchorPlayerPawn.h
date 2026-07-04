// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "AnchorPlayerPawn.generated.h"

/**
 * 玩家锚的占位 Pawn。
 * 另一位程序将接管真正的玩家移动、属性与失败判定。
 * 当前仅用于承载 Paper2D Sprite，方便在关卡中占位和测试相机。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API AAnchorPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	AAnchorPlayerPawn(const FObjectInitializer& ObjectInitializer);
	

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Pawn", meta = (ToolTip = "占位 Pawn 的 Sprite 组件，后续由正式玩家 Pawn 替换或继承。"))
	 TObjectPtr<class UPaperSpriteComponent> SpriteComponent;
};
