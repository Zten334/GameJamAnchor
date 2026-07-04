// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AnchorPlayerPawn.h"
#include "PaperSpriteComponent.h"

AAnchorPlayerPawn::AAnchorPlayerPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	SpriteComponent = ObjectInitializer.CreateDefaultSubobject<UPaperSpriteComponent>(this, TEXT("SpriteComponent"));
	RootComponent = SpriteComponent;

	// 美术资产就绪后，在蓝图子类中指定 Sprite。
	// 当前保留为空，使用引擎默认材质与占位 Sprite。
}
