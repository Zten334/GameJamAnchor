// Copyright Epic Games, Inc. All Rights Reserved.

#include "Chunk/ChunkManager.h"
#include "Chunk/Chunk.h"
#include "Framework/GameJamAnchorGameMode.h"
#include "Engine/World.h"

AChunkManager::AChunkManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void AChunkManager::BeginPlay()
{
	Super::BeginPlay();

	if (AGameJamAnchorGameMode* GameMode = Cast<AGameJamAnchorGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OnAnchorPlayerOutOfBounds.AddDynamic(this, &AChunkManager::OnPlayerOutOfBounds);
	}

	SpawnInitialChunks();
}

void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsScrolling)
	{
		return;
	}

	UpdateChunkPositions(DeltaTime);
	RecycleChunks();
	SpawnNewChunksIfNeeded();
}

void AChunkManager::OnPlayerOutOfBounds()
{
	bIsScrolling = false;
	SetActorTickEnabled(false);
	UE_LOG(LogTemp, Log, TEXT("ChunkManager: Player out of bounds, stopping chunk scroll."));
}

void AChunkManager::SpawnInitialChunks()
{
	float CurrentCenterZ = ScreenBottomZ + (ChunkHeight / 2.0f);
	TSubclassOf<AChunk> NextClass = SelectNextChunkClass();

	for (int32 i = 0; i < InitialChunkCount; ++i)
	{
		if (!NextClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("ChunkManager: No ChunkClass or ChunkClasses configured."));
			return;
		}

		SpawnChunkAtCenterZ(CurrentCenterZ, NextClass);
		if (bHasSpawnedTerminationChunk)
		{
			break;
		}

		TSubclassOf<AChunk> FollowingClass = SelectNextChunkClass();
		if (FollowingClass)
		{
			const float ExtraGap = GetDefaultTrailingGap(NextClass) + GetDefaultLeadingGap(FollowingClass);
			CurrentCenterZ -= (ChunkSpacing + ExtraGap);
		}
		NextClass = FollowingClass;
	}
}

void AChunkManager::SpawnChunkAtCenterZ(float CenterZ, TSubclassOf<AChunk> ClassToSpawn)
{
	if (!ClassToSpawn)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AChunk* NewChunk = GetWorld()->SpawnActor<AChunk>(ClassToSpawn, FVector(0.0f, 0.0f, CenterZ), FRotator::ZeroRotator, SpawnParams);
	if (!NewChunk)
	{
		return;
	}

	NewChunk->ChunkHeight = ChunkHeight;
	NewChunk->ChunkWidth = ChunkWidth;
	NewChunk->ScrollSpeed = ScrollSpeed;
	ActiveChunks.Add(NewChunk);
	++SpawnedChunkCount;

	if (TerminationChunkClass && ClassToSpawn == TerminationChunkClass)
	{
		bHasSpawnedTerminationChunk = true;
		UE_LOG(LogTemp, Log, TEXT("ChunkManager: spawned termination chunk (#%d)."), SpawnedChunkCount);
	}
}

void AChunkManager::UpdateChunkPositions(float DeltaTime)
{
	const float DeltaZ = ScrollSpeed * DeltaTime;
	for (AChunk* Chunk : ActiveChunks)
	{
		if (!Chunk)
		{
			continue;
		}

		Chunk->SyncScrollSpeed(ScrollSpeed);

		FVector Location = Chunk->GetActorLocation();
		Location.Z += DeltaZ;
		Chunk->SetActorLocation(Location);
	}
}

void AChunkManager::RecycleChunks()
{
	for (int32 i = ActiveChunks.Num() - 1; i >= 0; --i)
	{
		AChunk* Chunk = ActiveChunks[i];
		if (!Chunk)
		{
			ActiveChunks.RemoveAt(i);
			continue;
		}

		const float ChunkBottomZ = Chunk->GetActorLocation().Z - (ChunkHeight / 2.0f);
		if (ChunkBottomZ > ScreenTopZ)
		{
			Chunk->Destroy();
			ActiveChunks.RemoveAt(i);
		}
	}
}

void AChunkManager::SpawnNewChunksIfNeeded()
{
	if (bHasSpawnedTerminationChunk)
	{
		return;
	}

	AChunk* BottomChunk = nullptr;
	for (AChunk* Chunk : ActiveChunks)
	{
		if (Chunk && (!BottomChunk || Chunk->GetActorLocation().Z < BottomChunk->GetActorLocation().Z))
		{
			BottomChunk = Chunk;
		}
	}

	if (!BottomChunk)
	{
		SpawnChunkAtCenterZ(ScreenBottomZ + (ChunkHeight / 2.0f), SelectNextChunkClass());
		return;
	}

	while (BottomChunk->GetActorLocation().Z + (ChunkHeight / 2.0f) > ScreenBottomZ)
	{
		TSubclassOf<AChunk> NextClass = SelectNextChunkClass();
		if (!NextClass)
		{
			break;
		}

		const float ExtraGap = BottomChunk->TrailingGap + GetDefaultLeadingGap(NextClass);
		const float NewCenterZ = BottomChunk->GetActorLocation().Z - (ChunkSpacing + ExtraGap);
		SpawnChunkAtCenterZ(NewCenterZ, NextClass);

		if (bHasSpawnedTerminationChunk)
		{
			break;
		}

		BottomChunk = ActiveChunks.Last();
		if (!BottomChunk)
		{
			break;
		}
	}
}

TSubclassOf<AChunk> AChunkManager::SelectNextChunkClass() const
{
	if (TerminationChunkClass && TerminationChunkIndex > 0 && SpawnedChunkCount >= TerminationChunkIndex)
	{
		return TerminationChunkClass;
	}

	if (ChunkClasses.Num() > 0)
	{
		return ChunkClasses[FMath::RandRange(0, ChunkClasses.Num() - 1)];
	}

	return ChunkClass;
}

float AChunkManager::GetDefaultLeadingGap(TSubclassOf<AChunk> InChunkClass) const
{
	if (const AChunk* DefaultChunk = Cast<AChunk>(InChunkClass->GetDefaultObject()))
	{
		return DefaultChunk->LeadingGap;
	}
	return 0.0f;
}

float AChunkManager::GetDefaultTrailingGap(TSubclassOf<AChunk> InChunkClass) const
{
	if (const AChunk* DefaultChunk = Cast<AChunk>(InChunkClass->GetDefaultObject()))
	{
		return DefaultChunk->TrailingGap;
	}
	return 0.0f;
}
