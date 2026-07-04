// Copyright Epic Games, Inc. All Rights Reserved.

#include "Obstacle/Obstacle.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "Components/BoxComponent.h"

AObstacle::AObstacle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootScene);

	if (UPaperSprite* DefaultSprite = LoadObject<UPaperSprite>(nullptr, TEXT("/Engine/EditorResources/S_Actor")))
	{
		SpriteComponent->SetSprite(DefaultSprite);
	}
}

void AObstacle::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("Obstacle %s spawned at %s (Dynamic=%d)."), *GetName(), *GetActorLocation().ToString(), bDynamic);
}

void AObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Location = GetActorLocation();
	Location.Z += ScrollSpeed * DeltaTime;

	if (bDynamic)
	{
		SwayPhase += SwaySpeed * DeltaTime;
		Location.X = InitialX + FMath::Sin(SwayPhase) * SwayAmplitude;
	}

	SetActorLocation(Location);
}
