// Copyright Epic Games, Inc. All Rights Reserved.

#include "Framework/SoundManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void ASoundManager::PlaySoundAtLocation(USoundBase* Sound, FVector Location)
{
	if (!Sound)
	{
		return;
	}

	CleanupFinished();

	// 超出上限：停掉最早的那个
	if (ActiveComponents.Num() >= MaxConcurrentSounds)
	{
		UAudioComponent* Oldest = ActiveComponents[0];
		if (Oldest)
		{
			Oldest->Stop();
		}
		ActiveComponents.RemoveAt(0);
	}

	UAudioComponent* NewComp = UGameplayStatics::SpawnSoundAtLocation(
		GetWorld(),
		Sound,
		Location,
		FRotator::ZeroRotator,
		/* VolumeMultiplier = */ 1.0f,
		/* PitchMultiplier   = */ 1.0f,
		/* StartTime         = */ 0.0f
	);

	if (NewComp)
	{
		ActiveComponents.Add(NewComp);
	}
}

void ASoundManager::CleanupFinished()
{
	ActiveComponents.RemoveAll([](const TObjectPtr<UAudioComponent>& Comp)
	{
		return !Comp || !Comp->IsPlaying();
	});
}
