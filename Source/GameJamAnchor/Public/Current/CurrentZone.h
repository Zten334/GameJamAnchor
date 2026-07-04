// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CurrentZone.generated.h"

/**
 * 洋流区域。
 * 本身没有实体碰撞，但会对进入区域的玩家 Pawn 持续施加一个水平方向的力/速度偏移。
 * 具体的移动响应由玩家 Pawn 通过 IAnchorPlayerInterface::ApplyCurrentForce 实现。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API ACurrentZone : public AActor
{
	GENERATED_BODY()

public:
	ACurrentZone(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	/** 洋流推力（世界空间，cm/s^2 或速度偏移量，由 Pawn 解释）。X 正方向为右，负方向为左。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current", meta = (ToolTip = "洋流对玩家施加的力/速度偏移。X 正方向为右，负方向为左；Y/Z 通常保持 0。"))
	FVector CurrentForce = FVector(-300.0f, 0.0f, 0.0f);

	/** 是否只在玩家位于区域内时施加力。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current", meta = (ToolTip = "关闭时洋流会影响全图玩家（不推荐）。"))
	bool bRequireOverlap = true;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Current", meta = (ToolTip = "根场景组件。"))
	TObjectPtr<class USceneComponent> RootScene;

	/** 触发区域，玩家进入后受到洋流影响。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Current", meta = (ToolTip = "洋流影响范围。玩家进入该区域后持续受到 CurrentForce 影响。"))
	TObjectPtr<class UBoxComponent> TriggerBox;

	/** 对单个玩家应用洋流。 */
	void ApplyCurrentToActor(AActor* PlayerActor, float DeltaTime);
};
