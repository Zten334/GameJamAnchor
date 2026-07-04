// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Obstacle.generated.h"

/**
 * 障碍基类。静态障碍只随场景上移；动态障碍额外在 X 方向做正弦摆动。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API AObstacle : public AActor
{
	GENERATED_BODY()

public:
	AObstacle(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 是否动态障碍（会左右摆动）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle", meta = (ToolTip = "勾选后障碍会在 X 方向做正弦摆动，否则只随场景向上移动。"))
	bool bDynamic = false;

	/** 上移速度，由 ChunkManager/Chunk 在生成时同步。运行中请勿直接修改。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Obstacle", meta = (ToolTip = "障碍向上移动的速度，由 ChunkManager 通过 Chunk 自动同步，不需要手动修改。"))
	float ScrollSpeed = 200.0f;

	/** 动态障碍的摆动幅度（世界单位）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle", meta = (ToolTip = "动态障碍左右摆动的最大偏移距离（世界单位）。"))
	float SwayAmplitude = 50.0f;

	/** 动态障碍的摆动速度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle", meta = (ToolTip = "动态障碍摆动的速度，值越大摆动越快。"))
	float SwaySpeed = 2.0f;

	/** 初始 X 偏移，由 Chunk 在生成时设置。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Obstacle", meta = (ToolTip = "障碍生成时的初始 X 位置，由 Chunk 根据配置自动设置。"))
	float InitialX = 0.0f;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Obstacle", meta = (ToolTip = "根场景组件。"))
	TObjectPtr<class USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Obstacle", meta = (ToolTip = "障碍的 Sprite 显示组件，在蓝图子类中替换为美术 Sprite。"))
	TObjectPtr<class UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Obstacle", meta = (ToolTip = "障碍的碰撞体，用于与玩家 Pawn 做碰撞/重叠判定。"))
	TObjectPtr<class UBoxComponent> CollisionBox;

	float SwayPhase = 0.0f;
};
