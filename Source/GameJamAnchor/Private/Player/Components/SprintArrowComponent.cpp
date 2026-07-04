// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Components/SprintArrowComponent.h"


// Sets default values for this component's properties
USprintArrowComponent::USprintArrowComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void USprintArrowComponent::UpdateRotation(FVector Velocity, FVector CenterPoint)
{
	if (Velocity.IsNearlyZero())
	{
		return;
	}

	float Angle = FMath::Atan2(Velocity.X, Velocity.Z);
	float MinRad = FMath::DegreesToRadians(MinRotation);
	float MaxRad = FMath::DegreesToRadians(MaxRotation);

	if (CachedRadius < 0.0f)
	{
		CachedRadius = GetRelativeLocation().Length();
		if (CachedRadius < 1.0f)
		{
			CachedRadius = 80.0f;    // 兜底默认半径
		}
	}
	float R = CachedRadius;

	float CosVal = FMath::Cos(Angle);
	// 防止 Velocity 朝下时 Cos 为负，箭头翻到玩家上方
	if (CosVal < 0.0f)
	{
		CosVal = 0.0f;
	}

	// 位置：XZ 平面下半弧
	SetRelativeLocation(FVector(
		R * FMath::Sin(Angle),
		0.0f,
		-R * CosVal
	));

	// 自身旋转：对齐径向朝外（Sprite 默认向下，Angle 度直接绕 Y 转）
	SetRelativeRotation(FRotator(FMath::RadiansToDegrees(Angle), 0.0f, 0.0f));


}






