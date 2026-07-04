// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameJamAnchorGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAnchorPlayerOutOfBounds);

UCLASS(Blueprintable)
class GAMEJAMANCHOR_API AGameJamAnchorGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGameJamAnchorGameMode(const FObjectInitializer& ObjectInitializer);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** 玩家完全离开可视区域时触发。Pawn 可调用 ReportAnchorPlayerOutOfBounds 通知场景系统。 */
	UPROPERTY(BlueprintAssignable, Category = "Anchor Events", meta = (ToolTip = "玩家完全离开 180×320 可视区域时触发。Pawn 应调用 ReportAnchorPlayerOutOfBounds 来广播此事件。"))
	FOnAnchorPlayerOutOfBounds OnAnchorPlayerOutOfBounds;

	/** 由玩家 Pawn 调用，广播失败事件。 */
	UFUNCTION(BlueprintCallable, Category = "Anchor Events", meta = (ToolTip = "由玩家 Pawn 调用，用于通知场景系统玩家已失败。"))
	void ReportAnchorPlayerOutOfBounds();

protected:
	/** 关卡中放置的静态 2D 相机。GameMode 会在玩家加入后把视角切到该相机。 */
	UFUNCTION(BlueprintPure, Category = "Anchor Camera", meta = (ToolTip = "查找关卡中放置的 AnchorCamera，玩家加入后会把视角切换到该相机。"))
	class AAAnchorCamera* FindAnchorCamera() const;
};
