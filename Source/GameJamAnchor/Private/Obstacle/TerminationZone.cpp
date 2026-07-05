// Copyright Epic Games, Inc. All Rights Reserved.

#include "Obstacle/TerminationZone.h"
#include "Components/BoxComponent.h"
#include "Framework/GameJamAnchorGameMode.h"
#include "Chunk/ChunkManager.h"
#include "Obstacle/Obstacle.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"

ATerminationZone::ATerminationZone(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bDynamic = false;
	bOneWayMovement = false;
	bWanderInRadius = false;
	bScrollWithChunk = false;
	bDestroyWithOwnerChunk = true;
	bDestroyOnHit = false;
	bApplyEffectOnce = true;

	GoalZone = CreateDefaultSubobject<UBoxComponent>(TEXT("GoalZone"));
	GoalZone->SetupAttachment(RootComponent);
	GoalZone->SetCollisionProfileName(FName("OverlapAllDynamic"));
	GoalZone->SetGenerateOverlapEvents(true);
	GoalZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	GoalZone->OnComponentBeginOverlap.AddDynamic(this, &ATerminationZone::OnGoalBeginOverlap);

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ATerminationZone::BeginPlay()
{
	Super::BeginPlay();

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ATerminationZone::OnGoalBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bGoalReached || !IsPlayerActor(OtherActor))
	{
		return;
	}

	TriggerGoalReached();
}

void ATerminationZone::TriggerGoalReached()
{
	bGoalReached = true;

	UE_LOG(LogTemp, Log, TEXT("TerminationZone %s: player reached goal."), *GetName());

	if (AGameJamAnchorGameMode* GameMode = Cast<AGameJamAnchorGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->ReportPlayerReachedGoal();
	}

	ReceiveOnGoalReached();

	if (ReachedSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ReachedSound, GetActorLocation());
	}

	if (ReachedEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ReachedEffect, GetActorLocation());
	}

	if (bStopChunkScrollOnReached)
	{
		TArray<AActor*> FoundManagers;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AChunkManager::StaticClass(), FoundManagers);
		for (AActor* ManagerActor : FoundManagers)
		{
			if (AChunkManager* Manager = Cast<AChunkManager>(ManagerActor))
			{
				Manager->StopScrolling();
			}
		}

		for (TActorIterator<AObstacle> It(GetWorld()); It; ++It)
		{
			if (AObstacle* Obstacle = *It)
			{
				Obstacle->bScrollWithChunk = false;
				Obstacle->ScrollSpeed = 0.0f;
			}
		}
	}
}
