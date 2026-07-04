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

	float R = GetRelativeLocation().Length();

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

	// 自身旋转：绕 Y 轴指向速度方向
	float ArrowDeg = FMath::RadiansToDegrees(Angle);
	SetRelativeRotation(FRotator(ArrowDeg, 0.0f, 0.0f));
}






