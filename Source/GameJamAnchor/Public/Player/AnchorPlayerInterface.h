// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AnchorPlayerInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UAnchorPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 *
 * 玩家锚的公共接口。
 * 场景系统（障碍、Chunk 等）通过此接口查询玩家状态，避免直接依赖具体 Pawn 类。
 */
class GAMEJAMANCHOR_API IAnchorPlayerInterface
{
	GENERATED_BODY()

public:
	/** 玩家是否正在冲刺。 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Anchor Player")
	bool IsDashing() const;
	virtual bool IsDashing_Implementation() const { return false; }

	/** 当前冲刺方向（单位向量）。 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Anchor Player")
	FVector GetDashDirection() const;
	virtual FVector GetDashDirection_Implementation() const { return FVector::ZeroVector; }

	/** 应用洋流推力（世界空间，单位 cm/s^2 或 N，由 Pawn 自行解释）。 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Anchor Player")
	void ApplyCurrentForce(FVector ForcePerSecond);
	virtual void ApplyCurrentForce_Implementation(FVector ForcePerSecond) { }
};
