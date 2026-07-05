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
