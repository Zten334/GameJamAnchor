// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PaperFlipbook.h"
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

	/** 相对于 Chunk 中心的旋转（XZ 平面，即绕 Y 轴旋转）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "障碍生成后在 XZ 平面上的旋转角度（绕 Y 轴）。可用于让鱼、岩石等倾斜或倒过来。"))
	FRotator RelativeRotation = FRotator::ZeroRotator;

	/** 是否开启动态摆动。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "勾选后障碍会在 X 方向做正弦摆动。"))
	bool bDynamic = false;

	/** 动态摆动幅度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "动态障碍左右摆动的最大偏移距离（世界单位）。"))
	float SwayAmplitude = 50.0f;

	/** 动态障碍左右摆动的最大速度（cm/s）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "动态障碍左右摆动的最大速度（cm/s）。实际轨迹是正弦波，此值是 X 方向的峰值速度。"))
	float SwaySpeed = 100.0f;

	/** 是否使用单向移动（覆盖摆动）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "勾选后障碍会沿 X 方向持续单向移动，而不是正弦摆动。正速度向右，负速度向左。"))
	bool bOneWayMovement = false;

	/** 单向移动速度（世界单位/秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "单向移动时的 X 方向速度。建议配合 MaxLifeTime 或 MaxDistanceFromSpawn 使用。"))
	float OneWaySpeed = 100.0f;

	/** 是否在一定半径内随机游动（覆盖摆动和单向移动）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "勾选后障碍会在以出生点为中心的圆内随机游动。"))
	bool bWanderInRadius = false;

	/** 随机游动的半径。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "随机游动的活动半径。"))
	float WanderRadius = 100.0f;

	/** 随机游动的速度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "随机游动的移动速度。"))
	float WanderSpeed = 100.0f;

	/** 随机游动时每秒最大转向角度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "游动方向每秒随机变化的剧烈程度。"))
	float WanderTurnRate = 90.0f;

	/** 是否用本配置覆盖障碍蓝图自身的移动参数（Dynamic / OneWay / Wander / SwayAmplitude / SwaySpeed / OneWaySpeed / WanderRadius / WanderSpeed / WanderTurnRate）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "勾选后，Chunk 会用上面的移动参数覆盖障碍蓝图自身的设置；不勾选则保留障碍蓝图自己的移动设置。"))
	bool bOverrideMovementSettings = false;

	/** 是否水平镜像翻转该障碍实例。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "勾选后只镜像该 Chunk 里这个障碍实例，不影响蓝图本身。"))
	bool bMirrorX = false;

	/** 该障碍实例的视觉缩放倍数，会乘到蓝图原有缩放之上。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "只缩放该 Chunk 里的这个障碍实例。默认 (1,1,1) 保持蓝图原大小；(2,2,2) 放大一倍。"))
	FVector RelativeScale3D = FVector::OneVector;

	/** 障碍生成后使用的 Flipbook 动画。会覆盖障碍默认的 Flipbook 设置。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "为该障碍实例指定一个 PaperFlipbook 动画。设置后会自动应用到生成的障碍上。"))
	TObjectPtr<class UPaperFlipbook> Flipbook = nullptr;
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

	/** 本 Chunk 的全局顺序索引，由 ChunkManager 在生成时设置。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Chunk", meta = (ToolTip = "本 Chunk 的全局顺序索引，表示它是第几个生成的 Chunk。"))
	int32 ChunkIndex = -1;

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
