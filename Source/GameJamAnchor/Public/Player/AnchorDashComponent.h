// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AnchorDashComponent.generated.h"

/**
 * 冲刺组件。
 * 可附加到玩家 Pawn 上，提供冲刺状态、方向、冷却管理。
 * 具体的输入绑定由 Pawn 自行处理，Pawn 调用 StartDash 即可。
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAMEJAMANCHOR_API UAnchorDashComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAnchorDashComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 开始一次冲刺。Direction 应为世界空间单位向量。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Dash")
	void StartDash(const FVector& Direction);

	/** 是否正在冲刺中。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Dash")
	bool IsDashing() const { return bIsDashing; }

	/** 当前是否可以冲刺（不在冷却中）。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Dash")
	bool CanDash() const { return bCanDash; }

	/** 当前冲刺方向。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Dash")
	FVector GetDashDirection() const { return DashDirection; }

	/** 冲刺速度（cm/s）。默认 800 cm/s = 8 m/s。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Dash", meta = (ToolTip = "冲刺速度，单位 cm/s。默认 800 即 8 m/s。"))
	float DashSpeed = 800.0f;

	/** 冲刺持续时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Dash", meta = (ToolTip = "冲刺持续时长。"))
	float DashDuration = 0.2f;

	/** 冲刺冷却时间（秒）。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Dash", meta = (ToolTip = "两次冲刺之间的冷却时间。"))
	float DashCooldown = 1.0f;

protected:
	void EndDash();
	void ResetCooldown();

	UPROPERTY(BlueprintReadOnly, Category = "Anchor Dash")
	bool bIsDashing = false;

	UPROPERTY(BlueprintReadOnly, Category = "Anchor Dash")
	bool bCanDash = true;

	FVector DashDirection = FVector::ZeroVector;

	FTimerHandle DashTimerHandle;
	FTimerHandle CooldownTimerHandle;
};
