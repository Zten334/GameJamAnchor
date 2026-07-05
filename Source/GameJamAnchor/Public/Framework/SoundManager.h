// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SoundManager.generated.h"

class UAudioComponent;
class USoundBase;

/**
 * 全局音效管理器（GameState）。
 * 限制同时播放的音效数量，防止音效过多。
 * 在 GameMode 中把 GameStateClass 设为本类（或蓝图子类）即可生效。
 */
UCLASS(Blueprintable)
class GAMEJAMANCHOR_API ASoundManager : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** 同时最多播放几个音效。超出时停掉最早的。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound Manager")
	int32 MaxConcurrentSounds = 3;

	/**
	 * 在指定位置播放音效（受并发数限制）。
	 * @param Sound  要播放的音效资源。
	 * @param Location  世界坐标位置。
	 */
	UFUNCTION(BlueprintCallable, Category = "Sound Manager")
	void PlaySoundAtLocation(USoundBase* Sound, FVector Location);

private:
	/** 清理已播放完毕的 AudioComponent。 */
	void CleanupFinished();

	UPROPERTY()
	TArray<TObjectPtr<UAudioComponent>> ActiveComponents;
};
