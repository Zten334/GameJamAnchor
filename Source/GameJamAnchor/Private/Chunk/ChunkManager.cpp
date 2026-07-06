// Copyright Epic Games, Inc. All Rights Reserved.

#include "Chunk/ChunkManager.h"
#include "Chunk/Chunk.h"
#include "Background/Background.h"
#include "Framework/GameJamAnchorGameMode.h"
#include "Obstacle/Obstacle.h"
#include "Engine/World.h"
#include "EngineUtils.h"

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
		GameMode->OnAnchorPlayerDied.AddDynamic(this, &AChunkManager::OnPlayerDied);
	}

	SpawnInitialChunks();
}

void AChunkManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (AGameJamAnchorGameMode* GameMode = Cast<AGameJamAnchorGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GameMode->IsGameOver())
		{
			if (bIsScrolling)
			{
				StopScrolling();
			}
			return;
		}
	}

	if (!bIsScrolling)
	{
		return;
	}

	UpdateChunkPositions(DeltaTime);
	RecycleChunks();
	SpawnNewChunksIfNeeded();
	UpdateBackgroundPhase();
	StopScrollingIfTerminationDone();
}

void AChunkManager::OnPlayerOutOfBounds()
{
	StopScrolling();
	UE_LOG(LogTemp, Log, TEXT("ChunkManager: Player out of bounds, stopping chunk scroll."));
}

void AChunkManager::OnPlayerDied()
{
	StopScrolling();
	UE_LOG(LogTemp, Log, TEXT("ChunkManager: Player died, stopping chunk scroll."));
}

void AChunkManager::SpawnInitialChunks()
{
	float CurrentCenterZ = ScreenBottomZ + (ChunkHeight / 2.0f);

	for (int32 i = 0; i < InitialChunkCount; ++i)
	{
		TSubclassOf<AChunk> NextClass = SelectNextChunkClass();
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

		if (i == InitialChunkCount - 1)
		{
			break;
		}

		TSubclassOf<AChunk> FollowingClass = SelectNextChunkClass();
		if (!FollowingClass)
		{
			break;
		}

		const float ExtraGap = GetDefaultTrailingGap(NextClass) + GetDefaultLeadingGap(FollowingClass);
		CurrentCenterZ -= (ChunkSpacing + ExtraGap);
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
	NewChunk->ChunkIndex = SpawnedChunkCount;
	ActiveChunks.Add(NewChunk);
	++SpawnedChunkCount;

	UpdateBackgroundPhase();

	if (TerminationChunkClass && ClassToSpawn == TerminationChunkClass)
	{
		bHasSpawnedTerminationChunk = true;
		TerminationChunkInstance = NewChunk;
		UE_LOG(LogTemp, Log, TEXT("ChunkManager: spawned termination chunk (#%d)."), SpawnedChunkCount);
	}
}

void AChunkManager::UpdateChunkPositions(float DeltaTime)
{
	if (AGameJamAnchorGameMode* GameMode = Cast<AGameJamAnchorGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (GameMode->IsGameOver())
		{
			return;
		}
	}

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

TSubclassOf<AChunk> AChunkManager::SelectNextChunkClass()
{
	if (FixedIntroChunkIndex < FixedIntroChunkClasses.Num())
	{
		return FixedIntroChunkClasses[FixedIntroChunkIndex++];
	}

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

void AChunkManager::UpdateBackgroundPhase()
{
	if (BackgroundPhases.Num() == 0)
	{
		return;
	}

	AChunk* CurrentChunk = nullptr;
	for (AChunk* Chunk : ActiveChunks)
	{
		if (Chunk && (!CurrentChunk || Chunk->GetActorLocation().Z < CurrentChunk->GetActorLocation().Z))
		{
			CurrentChunk = Chunk;
		}
	}

	if (!CurrentChunk)
	{
		return;
	}

	const int32 CurrentChunkIndex = CurrentChunk->ChunkIndex;

	int32 BestIndex = INDEX_NONE;
	int32 BestStart = INT_MIN;
	for (int32 i = 0; i < BackgroundPhases.Num(); ++i)
	{
		const FBackgroundPhaseConfig& Phase = BackgroundPhases[i];
		if (Phase.StartChunkIndex <= CurrentChunkIndex && Phase.StartChunkIndex > BestStart)
		{
			BestStart = Phase.StartChunkIndex;
			BestIndex = i;
		}
	}

	if (BestIndex == INDEX_NONE || BestIndex == CurrentBackgroundPhaseIndex)
	{
		return;
	}

	CurrentBackgroundPhaseIndex = BestIndex;
	const FBackgroundPhaseConfig& Phase = BackgroundPhases[BestIndex];

	if (CurrentBackgroundActor)
	{
		CurrentBackgroundActor->Destroy();
		CurrentBackgroundActor = nullptr;
	}

	if (Phase.BackgroundClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		CurrentBackgroundActor = GetWorld()->SpawnActor<ABackground>(Phase.BackgroundClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		UE_LOG(LogTemp, Log, TEXT("ChunkManager: switched to background phase %d at playing chunk #%d."), BestIndex, CurrentChunkIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChunkManager: background phase %d has no BackgroundClass assigned."), BestIndex);
	}
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

void AChunkManager::StopScrolling()
{
	bIsScrolling = false;
	ScrollSpeed = 0.0f;
	SetActorTickEnabled(false);

	for (TActorIterator<AObstacle> It(GetWorld()); It; ++It)
	{
		if (AObstacle* Obstacle = *It)
		{
			Obstacle->Freeze();
			Obstacle->SetActorTickEnabled(false);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ChunkManager: scrolling stopped."));
}

void AChunkManager::StopScrollingIfTerminationDone()
{
	if (!bHasSpawnedTerminationChunk || !TerminationChunkInstance)
	{
		return;
	}

	const float ChunkBottomZ = TerminationChunkInstance->GetActorLocation().Z - (TerminationChunkInstance->ChunkHeight / 2.0f);
	if (ChunkBottomZ >= ScreenTopZ)
	{
		StopScrolling();
		UE_LOG(LogTemp, Log, TEXT("ChunkManager: termination chunk fully scrolled out, stopping scroll."));
	}
}
