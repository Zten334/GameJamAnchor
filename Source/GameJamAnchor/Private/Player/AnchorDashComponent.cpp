// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/AnchorDashComponent.h"

UAnchorDashComponent::UAnchorDashComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAnchorDashComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsDashing)
	{
		AActor* Owner = GetOwner();
		if (Owner)
		{
			const FVector Delta = DashDirection * DashSpeed * DeltaTime;
			Owner->AddActorWorldOffset(Delta, true);
		}
	}
}

void UAnchorDashComponent::StartDash(const FVector& Direction)
{
	if (!bCanDash || bIsDashing)
	{
		return;
	}

	DashDirection = Direction.GetSafeNormal();
	if (DashDirection.IsNearlyZero())
	{
		return;
	}

	bIsDashing = true;
	bCanDash = false;

	GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &UAnchorDashComponent::EndDash, DashDuration, false);
}

void UAnchorDashComponent::EndDash()
{
	bIsDashing = false;
	GetWorld()->GetTimerManager().SetTimer(CooldownTimerHandle, this, &UAnchorDashComponent::ResetCooldown, DashCooldown, false);
}

void UAnchorDashComponent::ResetCooldown()
{
	bCanDash = true;
}
