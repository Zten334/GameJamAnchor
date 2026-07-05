// Copyright Epic Games, Inc. All Rights Reserved.

#include "Current/CurrentZone.h"
#include "Components/BoxComponent.h"
#include "Player/AnchorPlayerInterface.h"

ACurrentZone::ACurrentZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetCollisionProfileName(FName("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ACurrentZone::BeginPlay()
{
	Super::BeginPlay();
}

void ACurrentZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bRequireOverlap)
	{
		TArray<AActor*> OverlappingActors;
		TriggerBox->GetOverlappingActors(OverlappingActors);

		for (AActor* Actor : OverlappingActors)
		{
			ApplyCurrentToActor(Actor, DeltaTime);
		}
	}
	else
	{
		// 全局影响：暂不实现，需要遍历所有实现接口的玩家。
	}
}

void ACurrentZone::ApplyCurrentToActor(AActor* PlayerActor, float DeltaTime)
{
	if (!PlayerActor)
	{
		return;
	}

	if (!PlayerActor->ActorHasTag(FName(TEXT("Anchor.Player"))))
	{
		return;
	}

	if (PlayerActor->GetClass()->ImplementsInterface(UAnchorPlayerInterface::StaticClass()))
	{
		IAnchorPlayerInterface::Execute_ApplyCurrentForce(PlayerActor, CurrentForce);
	}
}
