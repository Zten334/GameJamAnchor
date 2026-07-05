// Copyright Epic Games, Inc. All Rights Reserved.

#include "Background/Background.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "Engine/Texture2D.h"

#if WITH_EDITOR
#include "SpriteEditorOnlyTypes.h"
#endif

ABackground::ABackground(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetSpriteColor(BackgroundColor);

	if (BackgroundSprite)
	{
		SpriteComponent->SetSprite(BackgroundSprite);
	}
	else if (BackgroundTexture)
	{
		if (UPaperSprite* GeneratedSprite = CreateSpriteFromTexture(BackgroundTexture))
		{
			SpriteComponent->SetSprite(GeneratedSprite);
		}
	}
}

void ABackground::BeginPlay()
{
	Super::BeginPlay();

	if (!SpriteComponent->GetSprite())
	{
		UE_LOG(LogTemp, Warning, TEXT("Background %s has no sprite assigned. Set BackgroundSprite or BackgroundTexture."), *GetName());
		return;
	}

	FVector2D SourceSize = FVector2D::ZeroVector;
	if (UTexture2D* BakedTexture = SpriteComponent->GetSprite()->GetBakedTexture())
	{
		SourceSize = FVector2D(BakedTexture->GetSizeX(), BakedTexture->GetSizeY());
	}
	if (SourceSize.X > 0.0f && SourceSize.Y > 0.0f)
	{
		const FVector NewScale(BackgroundWidth / SourceSize.X, BackgroundHeight / SourceSize.Y, 1.0f);
		SpriteComponent->SetRelativeScale3D(NewScale);
	}

	UE_LOG(LogTemp, Log, TEXT("Background %s: sprite=%s, scale=%s."),
		*GetName(),
		*SpriteComponent->GetSprite()->GetName(),
		*SpriteComponent->GetRelativeScale3D().ToString());
}

UPaperSprite* ABackground::CreateSpriteFromTexture(UTexture2D* Texture)
{
#if WITH_EDITOR
	if (!Texture)
	{
		return nullptr;
	}

	UPaperSprite* Sprite = NewObject<UPaperSprite>(this);
	FSpriteAssetInitParameters InitParams;
	InitParams.SetTextureAndFill(Texture);
	InitParams.SetPixelsPerUnrealUnit(1.0f);
	Sprite->InitializeSprite(InitParams);

	return Sprite;
#else
	UE_LOG(LogTemp, Warning, TEXT("Background %s: BackgroundTexture is not supported in packaged builds. Assign a BackgroundSprite instead."), *GetName());
	return nullptr;
#endif
}
