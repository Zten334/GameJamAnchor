// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChunkManager.generated.h"

class ABackground;
class AChunk;

/**
 * 背景阶段配置。
 * 当“当前正在游玩的 Chunk”（即屏幕最下方仍在显示的最小编号 Chunk）达到 StartChunkIndex 时，切换到对应背景。
 */
USTRUCT(BlueprintType)
struct FBackgroundPhaseConfig
{
	GENERATED_BODY()

	/** 从第几个正在游玩的 Chunk 开始切换到这个背景（包含该 Chunk）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Background", meta = (ToolTip = "从第几个正在游玩的 Chunk 开始切换到这个背景。当前游玩的 Chunk 指屏幕最下方仍在显示的最小编号 Chunk。填 0 表示从开局就使用。"))
	int32 StartChunkIndex = 0;

	/** 该阶段使用的背景蓝图类。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Background", meta = (ToolTip = "该阶段要生成的背景蓝图类，例如 BP_Background_ShallowSea。"))
	TSubclassOf<class ABackground> BackgroundClass;
};

/**
 * Chunk 管理器：控制 320 高关卡段的生成、上移、销毁。
 * 向上移动模拟锚向下落；出屏后销毁，下方持续补充新 Chunk。
 * 支持固定前置 Chunk 序列（教学关）、多种普通 Chunk 模板循环、终点 Chunk，以及按阶段切换背景。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API AChunkManager : public AActor
{
	GENERATED_BODY()

public:
	AChunkManager(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	/** 已经生成的 Chunk 总数（含初始生成）。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Chunk", meta = (ToolTip = "已经生成的 Chunk 总数，包含初始生成和后续补充的。"))
	int32 SpawnedChunkCount = 0;

	/** 是否已经生成过终点 Chunk。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Chunk", meta = (ToolTip = "是否已经生成过终点 Chunk。生成后会停止继续补充新 Chunk。"))
	bool bHasSpawnedTerminationChunk = false;

	/** 停止 Chunk 滚动。会停止自身 Tick 并把 ScrollSpeed 置 0。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Chunk")
	void StopScrolling();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnPlayerOutOfBounds();

	UFUNCTION()
	void OnPlayerDied();

	bool bIsScrolling = true;

	/** 教学关/引导阶段使用的固定 Chunk 序列。按数组顺序生成，耗尽后才进入随机生成。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "教学关或引导阶段使用的固定 Chunk 序列。会按数组顺序依次生成，全部用完之后才会使用 ChunkClasses 随机生成。"))
	TArray<TSubclassOf<class AChunk>> FixedIntroChunkClasses;

	/** 单一 Chunk 模板（兼容旧用法）。如果 ChunkClasses 为空，会回退使用此项。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "单一 Chunk 模板。当 ChunkClasses 数组为空时，管理器会使用此项生成所有普通 Chunk。"))
	TSubclassOf<class AChunk> ChunkClass;

	/** 多种普通 Chunk 模板，会随机挑选生成。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "多种普通 Chunk 模板数组。设置后管理器会从中随机挑选生成，让关卡更丰富。"))
	TArray<TSubclassOf<class AChunk>> ChunkClasses;

	/** 终点 Chunk 模板。设置后，在生成指定数量的普通 Chunk 后生成。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "终点 Chunk 模板。玩家到达该 Chunk 后不再生成后续 Chunk。"))
	TSubclassOf<class AChunk> TerminationChunkClass;

	/** 在生成多少个普通 Chunk 后生成终点 Chunk。<= 0 表示无限循环。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "生成多少个普通 Chunk 后生成终点 Chunk。例如填 20，表示第 20 个普通 Chunk 之后生成终点 Chunk；填 -1 表示无限循环。"))
	int32 TerminationChunkIndex = -1;

	/** 按阶段切换背景。当“当前正在游玩的 Chunk”达到配置值时，生成对应的背景蓝图。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Background", meta = (ToolTip = "按阶段切换背景。当前游玩的 Chunk 指屏幕最下方仍在显示的最小编号 Chunk。例如 StartChunkIndex=5 表示第 5 个 Chunk 滚动到屏幕底部时切换背景。"))
	TArray<FBackgroundPhaseConfig> BackgroundPhases;

	/** Chunk 上移速度（世界单位/秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "场景整体向上滚动的速度（世界单位/秒）。所有 Chunk 和障碍会同步此速度。"))
	float ScrollSpeed = 200.0f;

	/** 单个 Chunk 高度，默认 320。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "单个 Chunk 的高度，默认 320，建议与相机逻辑高度一致。"))
	float ChunkHeight = 320.0f;

	/** 单个 Chunk 宽度，默认 180。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "单个 Chunk 的宽度，默认 180，建议与相机逻辑宽度一致。"))
	float ChunkWidth = 180.0f;

	/** 相邻两个 Chunk 中心之间的基础间距。默认等于 ChunkHeight（无缝拼接）。
	 *  实际间距还会受到前后 Chunk 的 LeadingGap / TrailingGap 影响。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "相邻两个 Chunk 中心之间的基础间距。默认 320（无缝拼接），调大可在 Chunk 之间留出间隙。实际间距还会加上前后 Chunk 的 LeadingGap / TrailingGap。"))
	float ChunkSpacing = 320.0f;

	/** 屏幕底部 Z 坐标，默认 -160（180×320 竖屏的一半高度）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "屏幕底部在世界空间中的 Z 坐标，默认 -160。与相机和玩家出生位置对应。"))
	float ScreenBottomZ = -160.0f;

	/** 屏幕顶部 Z 坐标，默认 160。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "屏幕顶部在世界空间中的 Z 坐标，默认 160。Chunk 底部超过此值后会被销毁。"))
	float ScreenTopZ = 160.0f;

	/** 初始生成 Chunk 数量，覆盖屏幕及下方缓冲。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Chunk", meta = (ToolTip = "开始时一次生成多少个 Chunk，用于覆盖屏幕及屏幕下方的缓冲区域。"))
	int32 InitialChunkCount = 3;

	UPROPERTY()
	TArray<TObjectPtr<AChunk>> ActiveChunks;

	void SpawnInitialChunks();
	void SpawnChunkAtCenterZ(float CenterZ, TSubclassOf<AChunk> ClassToSpawn);
	void UpdateChunkPositions(float DeltaTime);
	void RecycleChunks();
	void SpawnNewChunksIfNeeded();

	TSubclassOf<AChunk> SelectNextChunkClass();
	float GetDefaultLeadingGap(TSubclassOf<AChunk> InChunkClass) const;
	float GetDefaultTrailingGap(TSubclassOf<AChunk> InChunkClass) const;

	/** 检查并切换当前背景阶段。 */
	void UpdateBackgroundPhase();

	/** 固定前置序列的下一个索引。 */
	int32 FixedIntroChunkIndex = 0;

	/** 当前背景阶段索引。 */
	int32 CurrentBackgroundPhaseIndex = INDEX_NONE;

	/** 当前生成的背景 Actor。 */
	UPROPERTY()
	TObjectPtr<class ABackground> CurrentBackgroundActor = nullptr;

	/** 已生成的终点 Chunk 实例。用于判断终点 Chunk 是否完全出屏并停止滚动。 */
	UPROPERTY()
	TObjectPtr<class AChunk> TerminationChunkInstance = nullptr;

	/** 检查终点 Chunk 是否已完全出屏，若是则停止滚动。 */
	void StopScrollingIfTerminationDone();
};
