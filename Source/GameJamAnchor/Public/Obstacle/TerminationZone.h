// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Obstacle/Obstacle.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "TerminationZone.generated.h"

class UBoxComponent;

/**
 * 终点区域。
 * 放到 TerminationChunk 中，玩家进入后广播到达终点事件。
 * 本身没有实体碰撞，仅作为触发区域。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API ATerminationZone : public AObstacle
{
	GENERATED_BODY()

public:
	ATerminationZone(const FObjectInitializer& ObjectInitializer);

	/** 到达终点后是否停止场景滚动。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Termination", meta = (ToolTip = "勾选后玩家进入终点区域时会停止 ChunkManager 和所有障碍的滚动。"))
	bool bStopChunkScrollOnReached = true;

	/** 到达终点时播放的音效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Termination|Effects", meta = (ToolTip = "玩家进入终点区域时播放的音效。"))
	TObjectPtr<class USoundBase> ReachedSound = nullptr;

	/** 到达终点时生成的粒子特效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Termination|Effects", meta = (ToolTip = "玩家进入终点区域时生成的粒子特效。"))
	TObjectPtr<class UParticleSystem> ReachedEffect = nullptr;

	/** 终点触发区域。玩家进入后触发胜利事件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Termination|Components", meta = (ToolTip = "终点判定区域。玩家进入该区域后触发到达终点事件。"))
	TObjectPtr<UBoxComponent> GoalZone;

protected:
	virtual void BeginPlay() override;

	/** 是否已经触发过终点事件。 */
	bool bGoalReached = false;

	/** 玩家进入终点区域时的回调。 */
	UFUNCTION()
	void OnGoalBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 触发到达终点事件。 */
	void TriggerGoalReached();

	/** 到达终点时的蓝图事件。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Anchor Termination|Events", meta = (DisplayName = "On Goal Reached"))
	void ReceiveOnGoalReached();
};
