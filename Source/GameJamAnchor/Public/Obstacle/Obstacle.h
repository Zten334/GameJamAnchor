// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"
#include "PaperFlipbook.h"
#include "PaperFlipbookComponent.h"
#include "Obstacle.generated.h"

/**
 * 障碍基类。静态障碍只随场景上移；动态障碍额外在 X 方向做正弦摆动。
 * 支持不跟随 Chunk 滚动的独立障碍，并可通过生命周期或距离自动销毁。
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "勾选后障碍会在 X 方向做正弦摆动，否则只随场景向上移动。"))
	bool bDynamic = false;

	/** 是否单向移动（一直朝一个方向平移，不与 bDynamic 的摆动叠加）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "勾选后障碍会沿 X 方向持续单向移动；会覆盖 bDynamic 的摆动逻辑。正速度向右，负速度向左。"))
	bool bOneWayMovement = false;

	/** 是否在一定半径内随机游动（覆盖 Dynamic 和 OneWay）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "勾选后障碍会在以出生点为中心的圆内随机游动，覆盖 bDynamic 摆动和 bOneWayMovement 单向移动。"))
	bool bWanderInRadius = false;

	/** 随机游动的半径。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "随机游动的活动半径。"))
	float WanderRadius = 100.0f;

	/** 随机游动的速度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "随机游动的移动速度。"))
	float WanderSpeed = 100.0f;

	/** 随机游动时每秒最大转向角度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "游动方向每秒随机变化的剧烈程度，越大越飘忽。"))
	float WanderTurnRate = 90.0f;

	/** 单向移动速度（世界单位/秒）。正数向右，负数向左。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "单向移动时的 X 方向速度。正数向右，负数向左。建议配合 MaxLifeTime 或 MaxDistanceFromSpawn 使用，避免障碍无限远离。"))
	float OneWaySpeed = 100.0f;

	/** 是否跟随 Chunk 一起向上滚动。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "勾选时障碍会随场景一起向上滚动；不勾选时障碍生成后停留在世界空间中，不再跟随 Chunk 上移。"))
	bool bScrollWithChunk = true;

	/** 上移速度，由 ChunkManager/Chunk 在生成时同步。运行中请勿直接修改。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "障碍向上移动的速度，由 ChunkManager 通过 Chunk 自动同步，不需要手动修改。"))
	float ScrollSpeed = 200.0f;

	/** 动态障碍的摆动幅度（世界单位）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "动态障碍左右摆动的最大偏移距离（世界单位）。"))
	float SwayAmplitude = 50.0f;

	/** 动态障碍的左右移动最大速度（cm/s）。实际运动是正弦摆动，此值代表 X 方向峰值速度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "动态障碍左右摆动的最大速度（cm/s）。实际轨迹是正弦波，此值是 X 方向的峰值速度。幅度越大，要达到相同速度需要越大的值。"))
	float SwaySpeed = 100.0f;

	/** 初始 X 偏移，由 Chunk 在生成时设置。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor Obstacle|Movement", meta = (ToolTip = "障碍生成时的初始 X 位置，由 Chunk 根据配置自动设置。"))
	float InitialX = 0.0f;

	/** 最大存活时间（秒）。0 表示不限制。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Lifecycle", meta = (ToolTip = "障碍生成后的最大存活时间（秒）。到达后自动销毁；填 0 表示不限制。"))
	float MaxLifeTime = 0.0f;

	/** 离出生点的最大距离（世界单位）。0 表示不限制。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Lifecycle", meta = (ToolTip = "障碍离出生点的最大距离（世界单位）。超过后自动销毁；填 0 表示不限制。"))
	float MaxDistanceFromSpawn = 0.0f;

	/** 所属 Chunk 被销毁时，是否一并销毁本障碍。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Lifecycle", meta = (ToolTip = "所属 Chunk 被回收销毁时，是否一起销毁本障碍。不跟随 Chunk 滚动的独立障碍可以关闭此项以继续存活。"))
	bool bDestroyWithOwnerChunk = true;

	/** 是否使用 Sprite 自带的碰撞几何体作为碰撞体。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Collision", meta = (ToolTip = "勾选时使用 PaperSprite 自带的碰撞几何体（需在 Sprite 编辑器里设置），并关闭 BoxComponent。适用于不规则形状的障碍。"))
	bool bUseSpriteCollision = false;

	/** 命中玩家时触发的效果标签。同伴的 Pawn 会根据此标签处理减速、击退等效果。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Hit", meta = (ToolTip = "命中玩家时触发的效果标签，例如 Slow、Knockback、Damage。同伴的 Pawn 会监听 GameMode 的 OnPlayerHitObstacle 委托并据此处理。"))
	FName EffectTag;

	/** 命中玩家后是否销毁本障碍。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Hit", meta = (ToolTip = "命中玩家后是否销毁本障碍。例如小鱼可以销毁，礁石/岩壁则不销毁。"))
	bool bDestroyOnHit = false;

	/** 是否只触发一次效果（防止同一次接触重复触发）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Hit", meta = (ToolTip = "是否只触发一次效果。防止玩家在障碍上蹭来蹭去时反复触发。"))
	bool bApplyEffectOnce = true;

	/** 是否可以被冲刺击碎。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Dash", meta = (ToolTip = "勾选后，玩家处于冲刺状态撞到本障碍时会被击碎；否则按普通障碍处理（阻挡/触发 EffectTag）。"))
	bool bBreakableByDash = false;

	/** 被冲刺击碎后是否销毁本障碍。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Dash", meta = (ToolTip = "被冲刺击碎后是否直接销毁。如需播放断裂动画可关闭此项，在蓝图 ReceiveOnDashBroken 里手动处理。"))
	bool bDestroyOnDashBreak = true;

	/** 命中玩家时播放的音效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Effects", meta = (ToolTip = "命中玩家时在该位置播放的音效。"))
	TObjectPtr<class USoundBase> HitSound = nullptr;

	/** 命中玩家时生成的粒子特效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Effects", meta = (ToolTip = "命中玩家时在该位置生成的粒子特效。"))
	TObjectPtr<class UParticleSystem> HitEffect = nullptr;

	/** 障碍被销毁时播放的音效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Effects", meta = (ToolTip = "障碍被销毁时在该位置播放的音效。"))
	TObjectPtr<class USoundBase> DestroySound = nullptr;

	/** 障碍被销毁时生成的粒子特效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Effects", meta = (ToolTip = "障碍被销毁时在该位置生成的粒子特效。"))
	TObjectPtr<class UParticleSystem> DestroyEffect = nullptr;

	/** 障碍生成时播放的音效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Effects", meta = (ToolTip = "障碍生成时在该位置播放的音效。"))
	TObjectPtr<class USoundBase> SpawnSound = nullptr;

	/** 障碍生成时生成的粒子特效。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Effects", meta = (ToolTip = "障碍生成时在该位置生成的粒子特效，可用于出生提示、气泡、烟尘等。"))
	TObjectPtr<class UParticleSystem> SpawnEffect = nullptr;

	/** 障碍使用的 Flipbook 动画。设置后会自动隐藏静态 Sprite 并播放此动画。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Visual", meta = (ToolTip = "障碍使用的 PaperFlipbook 动画。设置后运行时会自动隐藏 SpriteComponent、显示并播放 Flipbook。不设置则保持静态 Sprite。"))
	TObjectPtr<class UPaperFlipbook> Flipbook = nullptr;

	/** 精灵默认是否朝右。勾选时认为 Sprite/Flipbook 原图朝右，向左移动时会自动水平翻转。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Visual", meta = (ToolTip = "勾选时认为 Sprite/Flipbook 原图朝右；向左移动时组件 Scale.X 会自动变负以调头。若原图朝左则取消勾选。"))
	bool bFaceRightByDefault = true;

	/** 是否水平镜像翻转视觉组件（不影响碰撞体）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Visual", meta = (ToolTip = "勾选后该障碍实例的视觉组件会在原图基础上额外做一次水平镜像。可用于同一条鱼蓝图生成方向相反的实例。"))
	bool bMirrorX = false;

	/** 视觉组件的缩放倍数，会乘到蓝图原有缩放之上。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Visual", meta = (ToolTip = "对 Sprite/Flipbook 组件的缩放倍数，默认 (1,1,1)。例如 (2,2,2) 放大一倍，(0.5,0.5,0.5) 缩小一半。"))
	FVector RelativeScale3D = FVector::OneVector;

	/** 是否显示悬浮警示图标。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Warning", meta = (ToolTip = "勾选后在障碍上方显示一个警示 Sprite，例如感叹号、危险标记等。"))
	bool bShowWarning = false;

	/** 警示图标使用的 PaperSprite。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Warning", meta = (ToolTip = "悬浮在障碍上方的警示图标 Sprite。"))
	TObjectPtr<class UPaperSprite> WarningSprite = nullptr;

	/** 警示图标相对障碍的位置偏移。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Warning", meta = (ToolTip = "警示图标相对障碍的位置偏移。默认 (0, 0, 50) 即在头顶 50 单位。Y 值可调整图层深度。"))
	FVector WarningOffset = FVector(0.0f, 0.0f, 50.0f);

	/** 警示图标的额外缩放。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Warning", meta = (ToolTip = "警示图标的缩放倍数，默认 1。"))
	float WarningSpriteScale = 1.0f;

	/** 由 Chunk 生成后调用，应用配置中的旋转/镜像/缩放等视觉设置。 */
	void ApplyVisualConfig();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Obstacle|Components", meta = (ToolTip = "根场景组件。"))
	TObjectPtr<class USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Obstacle|Components", meta = (ToolTip = "障碍的 Sprite 显示组件，在蓝图子类中替换为美术 Sprite。"))
	TObjectPtr<class UPaperSpriteComponent> SpriteComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Obstacle|Components", meta = (ToolTip = "障碍的碰撞体，用于与玩家 Pawn 做碰撞/重叠判定。"))
	TObjectPtr<class UBoxComponent> CollisionBox;

	/** 障碍的 Flipbook 动画组件。设置了 Flipbook 资产后会自动启用。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Obstacle|Components", meta = (ToolTip = "障碍的 Flipbook 动画组件。在 Details 面板中指定 Flipbook 资产后，运行时会自动播放动画。"))
	TObjectPtr<class UPaperFlipbookComponent> FlipbookComponent;

	/** 悬浮警示图标组件。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor Obstacle|Components", meta = (ToolTip = "悬浮在障碍上方的警示图标组件。"))
	TObjectPtr<class UPaperSpriteComponent> WarningSpriteComponent;

	float SwayPhase = 0.0f;

	/** 随机游动当前方向角（弧度）。 */
	float WanderAngle = 0.0f;

	/** 随机游动目标点。 */
	FVector WanderTarget = FVector::ZeroVector;

	/** 随机游动目标点剩余切换时间。 */
	float WanderTargetTimer = 0.0f;

	/** 视觉组件初始 X 缩放绝对值，用于调头时保持原尺寸。 */
	float InitialVisualScaleX = 1.0f;

	/** 视觉组件蓝图原有缩放，用于外部配置覆盖后恢复基准。 */
	FVector BaseVisualScale3D = FVector::OneVector;

	/** 出生点位置。 */
	FVector SpawnLocation = FVector::ZeroVector;

	/** 已存活时间。 */
	float ElapsedLifeTime = 0.0f;

	/** 检查并执行生命周期 / 距离销毁。 */
	void CheckDestroyConditions();

	/** 碰撞/重叠命中玩家时的回调。 */
	UFUNCTION()
	void OnHitPlayer(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** 判断碰撞对象是否是玩家 Pawn（通过 Tag "Anchor.Player"）。 */
	bool IsPlayerActor(AActor* Actor) const;

	/** 是否已经对当前玩家触发过效果。 */
	bool bEffectAlreadyApplied = false;

	/** 被冲刺击碎时的处理。蓝图可在 ReceiveOnDashBroken 里播放断裂动画。 */
	void OnDashBroken();

	/** 被冲刺击碎时的蓝图事件。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Anchor Obstacle|Events", meta = (DisplayName = "On Dash Broken"))
	void ReceiveOnDashBroken();

	/** 命中玩家时的蓝图事件（非冲刺击碎路径）。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Anchor Obstacle|Events", meta = (DisplayName = "On Hit Player"))
	void ReceiveOnHitPlayer();

	/** 障碍被销毁时的蓝图事件。 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Anchor Obstacle|Events", meta = (DisplayName = "On Destroyed"))
	void ReceiveOnDestroyed();

	/** 播放命中音效/特效。 */
	void PlayHitEffects();

	/** 播放销毁音效/特效。 */
	void PlayDestroyEffects();

	/** 根据移动方向水平翻转视觉组件（Sprite 或 Flipbook）。 */
	void UpdateVisualFacing(bool bMovingRight);

	/** 设置/刷新悬浮警示图标的可见性、位置、缩放和 Sprite。 */
	void UpdateWarningVisual();

	/** 检测碰撞对象是否正在冲刺。 */
	bool IsPlayerDashing(AActor* Actor) const;
};
