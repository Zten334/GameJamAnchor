// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameJamAnchorGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnchorPlayerOutOfBounds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAnchorPlayerHitObstacle, AActor*, Hitter, FName, EffectTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnchorPlayerReachedGoal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnchorPlayerDied);

UCLASS(Blueprintable)
class GAMEJAMANCHOR_API AGameJamAnchorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGameJamAnchorGameMode(const FObjectInitializer& ObjectInitializer);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** 玩家完全离开可视区域时触发。Pawn 可调用 ReportAnchorPlayerOutOfBounds 通知场景系统。 */
	UPROPERTY(BlueprintAssignable, Category = "Anchor Events", meta = (ToolTip = "玩家完全离开 180×320 可视区域时触发。Pawn 应调用 ReportAnchorPlayerOutOfBounds 来广播此事件。"))
	FOnAnchorPlayerOutOfBounds OnAnchorPlayerOutOfBounds;

	/** 由玩家 Pawn 调用，广播失败事件。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Events", meta = (ToolTip = "由玩家 Pawn 调用，用于通知场景系统玩家已失败。"))
	void ReportAnchorPlayerOutOfBounds();

	/** 玩家撞到障碍时触发。AObstacle 会调用 ReportPlayerHitObstacle 广播。 */
	UPROPERTY(BlueprintAssignable, Category = "Anchor Events", meta = (ToolTip = "玩家撞到障碍时触发。事件会携带造成撞击的障碍实例和 EffectTag，由 Pawn 根据标签处理减速/击退等效果。"))
	FOnAnchorPlayerHitObstacle OnPlayerHitObstacle;

	/** 由 AObstacle 调用，广播玩家撞到障碍的事件。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Events", meta = (ToolTip = "由障碍调用，用于通知玩家 Pawn 玩家被撞到了，并告知效果标签。"))
	void ReportPlayerHitObstacle(AActor* Hitter, FName EffectTag);

	/** 玩家到达终点区域时触发。ATerminationZone 会调用 ReportPlayerReachedGoal 广播。 */
	UPROPERTY(BlueprintAssignable, Category = "Anchor Events", meta = (ToolTip = "玩家到达终点区域时触发。可在蓝图里绑定以播放胜利动画、切换关卡等。"))
	FOnAnchorPlayerReachedGoal OnAnchorPlayerReachedGoal;

	/** 由 ATerminationZone 调用，广播玩家到达终点事件。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Events", meta = (ToolTip = "由终点区域调用，通知游戏玩家已到达终点。"))
	void ReportPlayerReachedGoal();

	/** 玩家死亡时触发（被障碍击杀、摔死等）。Pawn 调用 ReportPlayerDied 通知场景系统停止滚动。 */
	UPROPERTY(BlueprintAssignable, Category = "Anchor Events", meta = (ToolTip = "玩家死亡时触发。由 Pawn 调用，用于停止场景滚动等全局处理。"))
	FOnAnchorPlayerDied OnAnchorPlayerDied;

	/** 由玩家 Pawn 调用，广播死亡事件。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Events", meta = (ToolTip = "由玩家 Pawn 调用，通知场景系统玩家已死亡。"))
	void ReportPlayerDied();

	/** 返回游戏是否已经结束（死亡、出界或胜利）。 */
	UFUNCTION(BlueprintPure, Category = "Anchor State", meta = (ToolTip = "游戏是否已经结束。"))
	bool IsGameOver() const { return bIsGameOver; }

protected:
	/** 游戏是否已经结束。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor State")
	bool bIsGameOver = false;

	/** 关卡中放置的静态 2D 相机。GameMode 会在玩家加入后把视角切到该相机。 */
	UFUNCTION(BlueprintPure, Category = "Anchor Camera", meta = (ToolTip = "查找关卡中放置的 AnchorCamera，玩家加入后会把视角切换到该相机。"))
	class AAnchorCamera* FindAnchorCamera() const;
};
