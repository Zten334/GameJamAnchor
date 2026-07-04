// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chunk.generated.h"

class AObstacle;

/**
 * 单个障碍生成配置。
 */
USTRUCT(BlueprintType)
struct FObstacleSpawnConfig
{
	GENERATED_BODY()

	/** 要生成的障碍类。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "要生成的障碍蓝图类，例如 BP_Obstacle。"))
	TSubclassOf<AObstacle> ObstacleClass;

	/** 相对于 Chunk 中心的位置。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "障碍相对于 Chunk 中心的位置。X 为左右，Z 为上下（一般保持在 0 附近）。"))
	FVector RelativeLocation = FVector::ZeroVector;

	/** 是否开启动态摆动。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "勾选后障碍会在 X 方向做正弦摆动。"))
	bool bDynamic = false;

	/** 动态摆动幅度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "动态障碍左右摆动的最大偏移距离（世界单位）。"))
	float SwayAmplitude = 50.0f;

	/** 动态摆动速度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "动态障碍摆动的速度，值越大摆动越快。"))
	float SwaySpeed = 2.0f;
};

/**
 * 一个 320 单位高的关卡段（Chunk）。
 * 本身在游戏中不可见，负责按配置生成障碍。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API AChunk : public AActor
{
	GENERATED_BODY()

public:
	AChunk(const FObjectInitializer& ObjectInitializer);

	/** Chunk 在世界空间中的高度（默认 320，与相机逻辑高度一致）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "Chunk 在世界空间中的高度，默认 320，建议与相机逻辑高度一致。"))
	float ChunkHeight = 320.0f;

	/** Chunk 在世界空间中的宽度（默认 180，与相机逻辑宽度一致）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "Chunk 在世界空间中的宽度，默认 180，建议与相机逻辑宽度一致。"))
	float ChunkWidth = 180.0f;

	/** 上移速度，由 ChunkManager 在生成时同步。运行中请勿直接修改。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Chunk", meta = (ToolTip = "Chunk 的上移速度，由 ChunkManager 自动同步，不需要手动修改。"))
	float ScrollSpeed = 200.0f;

	/** 本 Chunk 包含的障碍生成配置。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "本 Chunk 需要生成的障碍列表。每个元素指定一种障碍类及其相对位置和动态参数。"))
	TArray<FObstacleSpawnConfig> ObstacleConfigs;

	/** 本 Chunk 与上一个 Chunk 之间的额外间距。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "本 Chunk 与上一个 Chunk 之间的额外间距，用于控制关卡节奏。0 表示没有额外间距。"))
	float LeadingGap = 0.0f;

	/** 本 Chunk 与下一个 Chunk 之间的额外间距。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "本 Chunk 与下一个 Chunk 之间的额外间距，用于控制关卡节奏。0 表示没有额外间距。"))
	float TrailingGap = 0.0f;

	/** 同步本 Chunk 及其生成的障碍的上移速度。 */
	void SyncScrollSpeed(float NewScrollSpeed);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Chunk", meta = (ToolTip = "根场景组件。"))
	TObjectPtr<class USceneComponent> RootScene;

	/** 编辑器可视化组件，游戏中隐藏。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Chunk", meta = (ToolTip = "编辑器中用于可视化 Chunk 范围的 Sprite，运行时会自动隐藏。"))
	TObjectPtr<class UPaperSpriteComponent> SpriteComponent;

	/** 本 Chunk 生成的障碍实例，用于同步速度等参数。 */
	UPROPERTY()
	TArray<TObjectPtr<class AObstacle>> SpawnedObstacles;

	void SpawnObstacles();
};
