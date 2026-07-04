// Copyright Epic Games, Inc. All Rights Reserved.

#include "Framework/GameJamAnchorGameMode.h"
#include "Camera/AnchorCamera.h"
#include "Player/AnchorPlayerPawn.h"
#include "Kismet/GameplayStatics.h"

AGameJamAnchorGameMode::AGameJamAnchorGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 当前使用占位 Pawn，方便相机与输入系统工作。
	// 后续由另一位程序替换为真正的玩家 Anchor Pawn（在 .uproject/DefaultGame.ini 或蓝图 GameMode 中覆盖）。
	DefaultPawnClass = AAAnchorPlayerPawn::StaticClass();
}

void AGameJamAnchorGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (AAAnchorCamera* AnchorCamera = FindAnchorCamera())
	{
		NewPlayer->SetViewTargetWithBlend(AnchorCamera, 0.0f);
	}
}

void AGameJamAnchorGameMode::ReportAnchorPlayerOutOfBounds()
{
	UE_LOG(LogTemp, Log, TEXT("GameMode: Player out of bounds. Broadcasting failure event."));
	OnAnchorPlayerOutOfBounds.Broadcast();
}

AAAnchorCamera* AGameJamAnchorGameMode::FindAnchorCamera() const
{
	TArray<AActor*> FoundCameras;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAAnchorCamera::StaticClass(), FoundCameras);

	if (FoundCameras.Num() > 0)
	{
		return Cast<AAAnchorCamera>(FoundCameras[0]);
	}

	return nullptr;
}
