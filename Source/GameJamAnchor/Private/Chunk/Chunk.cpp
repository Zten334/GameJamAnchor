// Copyright Epic Games, Inc. All Rights Reserved.

#include "Chunk/Chunk.h"
#include "Obstacle/Obstacle.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "Engine/World.h"

AChunk::AChunk(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetHiddenInGame(true);

	if (UPaperSprite* DefaultSprite = LoadObject<UPaperSprite>(nullptr, TEXT("/Engine/EditorResources/S_Actor")))
	{
		SpriteComponent->SetSprite(DefaultSprite);
	}
}

void AChunk::BeginPlay()
{
	Super::BeginPlay();
	SpawnObstacles();
}

void AChunk::SyncScrollSpeed(float NewScrollSpeed)
{
	ScrollSpeed = NewScrollSpeed;
	for (AObstacle* Obstacle : SpawnedObstacles)
	{
		if (Obstacle)
		{
			Obstacle->ScrollSpeed = NewScrollSpeed;
		}
	}
}

void AChunk::SpawnObstacles()
{
	int32 SpawnedCount = 0;
	for (const FObstacleSpawnConfig& Config : ObstacleConfigs)
	{
		if (!Config.ObstacleClass)
		{
			continue;
		}

		const FVector SpawnLocation = GetActorLocation() + Config.RelativeLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AObstacle* NewObstacle = GetWorld()->SpawnActor<AObstacle>(Config.ObstacleClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
		if (!NewObstacle)
		{
			continue;
		}

		NewObstacle->ScrollSpeed = ScrollSpeed;
		NewObstacle->InitialX = SpawnLocation.X;
		NewObstacle->bDynamic = Config.bDynamic;
		NewObstacle->SwayAmplitude = Config.SwayAmplitude;
		NewObstacle->SwaySpeed = Config.SwaySpeed;
		SpawnedObstacles.Add(NewObstacle);
		++SpawnedCount;
	}

	UE_LOG(LogTemp, Log, TEXT("Chunk %s spawned %d obstacles."), *GetName(), SpawnedCount);
}
