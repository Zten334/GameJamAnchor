// Copyright Epic Games, Inc. All Rights Reserved.

#include "Framework/GameJamAnchorGameMode.h"
#include "Framework/SoundManager.h"
#include "Camera/AnchorCamera.h"
#include "Player/AnchorPlayerPawn.h"
#include "Kismet/GameplayStatics.h"

AGameJamAnchorGameMode::AGameJamAnchorGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DefaultPawnClass = AAnchorPlayerPawn::StaticClass();
	GameStateClass = ASoundManager::StaticClass();
}

void AGameJamAnchorGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AAnchorCamera* AnchorCamera = FindAnchorCamera())
	{
		NewPlayer->SetViewTargetWithBlend(AnchorCamera, 0.0f);
	}
}

void AGameJamAnchorGameMode::ReportAnchorPlayerOutOfBounds()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Player out of bounds. Broadcasting failure event."));
	bIsGameOver = true;
	OnAnchorPlayerOutOfBounds.Broadcast();
}

void AGameJamAnchorGameMode::ReportPlayerHitObstacle(AActor* Hitter, FName EffectTag)
{
	if (!Hitter)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("GameMode: Player hit obstacle %s with effect %s."), *Hitter->GetName(), *EffectTag.ToString());
	OnPlayerHitObstacle.Broadcast(Hitter, EffectTag);
}

void AGameJamAnchorGameMode::ReportPlayerReachedGoal()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Player reached goal. Broadcasting victory event."));
	bIsGameOver = true;
	OnAnchorPlayerReachedGoal.Broadcast();
}

void AGameJamAnchorGameMode::ReportPlayerDied()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Player died. Broadcasting death event."));
	bIsGameOver = true;
	OnAnchorPlayerDied.Broadcast();
}

AAnchorCamera* AGameJamAnchorGameMode::FindAnchorCamera() const
{
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAnchorCamera::StaticClass(), FoundCameras);

	if (FoundCameras.Num() > 0)
	{
		return Cast<AAnchorCamera>(FoundCameras[0]);
	}

	return nullptr;
}
