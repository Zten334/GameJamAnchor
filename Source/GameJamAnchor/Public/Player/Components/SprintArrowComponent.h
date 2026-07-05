// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteComponent.h"
#include "SprintArrowComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEJAMANCHOR_API USprintArrowComponent : public UPaperSpriteComponent
{
	GENERATED_BODY()

public:
	USprintArrowComponent();
/**
	 * 根据速度方向设置箭头旋转。
	 * 计算速度在 XZ 平面的方向角，钳制后应用为相对旋转。
	 */
	UFUNCTION(BlueprintCallable, Category = "SprintArrow")
	void UpdateRotation(FVector Velocity, FVector CenterPoint);
protected:
	UFUNCTION(BlueprintCallable, Category = "SprintArrow")
	void ShowArrow() { SetVisibility(true); }

	UFUNCTION(BlueprintCallable, Category = "SprintArrow")
	void HideArrow() { SetVisibility(false); }
	// ── 配置 ──
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SprintArrow")
	float MinRotation = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SprintArrow")
	float MaxRotation = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SprintArrow")
	float RotationOffset = 0.0f;

private:
	float CachedRadius = -1.0f;
};
