// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Obstacle/Obstacle.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "CurrentJetObstacle.generated.h"

class UBoxComponent;

/**
 * 洋流喷射障碍。
 * 从屏幕左/右边缘周期性喷出一道横向水流，覆盖整个可视宽度，并随 Chunk 向上移动。
 * 喷射期间进入触发区域的玩家会受到持续推力，并触发一次 EffectTag 广播。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API ACurrentJetObstacle : public AObstacle
{
	GENERATED_BODY()

public:
	ACurrentJetObstacle(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	/** 洋流推力（cm/s^2 或速度偏移，由 Pawn 解释）。X 正方向向右，负方向向左。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current Jet", meta = (ToolTip = "洋流对玩家施加的推力。X 正方向向右，负方向向左。例如 -300 表示向左推。"))
	FVector CurrentForce = FVector(-300.0f, 0.0f, 0.0f);

	/** 每次喷射的持续时间。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current Jet", meta = (ToolTip = "每次喷射持续多少秒。"))
	float JetDuration = 2.0f;

	/** 两次喷射之间的冷却时间。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current Jet", meta = (ToolTip = "喷射结束后等待多少秒再开始下一次。"))
	float JetCooldown = 3.0f;

	/** 生成后多久开始第一次喷射。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current Jet", meta = (ToolTip = "障碍生成后延迟多少秒开始第一次喷射。填 0 表示立刻开始。"))
	float JetStartupDelay = 0.0f;

	/** 喷射触发盒。水流本身没有实体阻挡，用 Overlap 检测玩家。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Current Jet|Components", meta = (ToolTip = "洋流触发盒。喷射期间启用，玩家进入后受到 CurrentForce 推力。"))
	TObjectPtr<UBoxComponent> CurrentTriggerBox;

	/** 开始喷射时播放的音效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current Jet|Effects", meta = (ToolTip = "每次开始喷射时播放一次的音效。"))
	TObjectPtr<USoundBase> JetStartSound = nullptr;

	/** 开始喷射时生成的粒子特效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current Jet|Effects", meta = (ToolTip = "每次开始喷射时生成的粒子特效。建议粒子生命周期覆盖整个 JetDuration，或在蓝图中手动控制。"))
	TObjectPtr<UParticleSystem> JetStartEffect = nullptr;

	/** 结束喷射时播放的音效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current Jet|Effects", meta = (ToolTip = "每次结束喷射时播放一次的音效。"))
	TObjectPtr<USoundBase> JetEndSound = nullptr;

	/** 结束喷射时生成的粒子特效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Current Jet|Effects", meta = (ToolTip = "每次结束喷射时生成的粒子特效。"))
	TObjectPtr<UParticleSystem> JetEndEffect = nullptr;

protected:
	virtual void BeginPlay() override;

	/** 当前是否正在喷射。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Current Jet")
	bool bIsJetting = false;

	/** 本次喷射剩余时间。 */
	float JetTimer = 0.0f;

	/** 当前冷却剩余时间。 */
	float CooldownTimer = 0.0f;

	/** 当前在洋流区域内并受到推力的玩家。 */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> AffectedPlayers;

	/** 已触发过 EffectTag 的玩家（本周期内）。 */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> TaggedPlayersThisCycle;

	/** 开始一次喷射。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Current Jet")
	void StartJet();

	/** 结束本次喷射。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Current Jet")
	void StopJet();

	/** 对当前在触发区域内的玩家应用推力。 */
	void ApplyCurrentToOverlappingPlayers(float DeltaTime);

	/** 玩家进入洋流触发区域时的回调。 */
	UFUNCTION()
	void OnCurrentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 玩家离开洋流触发区域时的回调。 */
	UFUNCTION()
	void OnCurrentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** 对单个玩家施加推力并触发 EffectTag。 */
	void ApplyCurrentToPlayer(AActor* PlayerActor);
};
