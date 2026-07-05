// Copyright Epic Games, Inc. All Rights Reserved.

#include "Obstacle/Obstacle.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "PaperFlipbookComponent.h"
#include "Components/BoxComponent.h"
#include "Framework/GameJamAnchorGameMode.h"
#include "Player/AnchorPlayerInterface.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"

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

	FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("FlipbookComponent"));
	FlipbookComponent->SetupAttachment(RootScene);
	FlipbookComponent->SetVisibility(false);
	FlipbookComponent->SetHiddenInGame(true);

	if (UPaperSprite* DefaultSprite = LoadObject<UPaperSprite>(nullptr, TEXT("/Engine/EditorResources/S_Actor")))
	{
		SpriteComponent->SetSprite(DefaultSprite);
	}
}

void AObstacle::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();

	UPrimitiveComponent* HitComponent = CollisionBox.Get();
	if (bUseSpriteCollision && SpriteComponent)
	{
		SpriteComponent->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
		SpriteComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		SpriteComponent->SetGenerateOverlapEvents(true);

		if (CollisionBox)
		{
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		HitComponent = SpriteComponent.Get();
	}

	if (HitComponent)
	{
		HitComponent->OnComponentHit.AddDynamic(this, &AObstacle::OnHitPlayer);
	}

	if (FlipbookComponent)
	{
		UPaperFlipbook* EffectiveFlipbook = Flipbook ? Flipbook.Get() : FlipbookComponent->GetFlipbook();
		if (EffectiveFlipbook && EffectiveFlipbook->GetNumKeyFrames() > 0)
		{
			FlipbookComponent->SetFlipbook(EffectiveFlipbook);
			FlipbookComponent->SetVisibility(true);
			FlipbookComponent->SetHiddenInGame(false);
			FlipbookComponent->Activate();
			FlipbookComponent->Play();
			FlipbookComponent->MarkRenderStateDirty();

			if (SpriteComponent)
			{
				SpriteComponent->SetVisibility(false);
				SpriteComponent->SetHiddenInGame(true);
			}

			UE_LOG(LogTemp, Log, TEXT("Obstacle %s activated Flipbook %s (frames=%d, sprite=%s)."),
				*GetName(), *EffectiveFlipbook->GetName(), EffectiveFlipbook->GetNumKeyFrames(),
				EffectiveFlipbook->GetKeyFrameChecked(0).Sprite ? *EffectiveFlipbook->GetKeyFrameChecked(0).Sprite->GetName() : TEXT("NULL"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Obstacle %s has FlipbookComponent but Flipbook is %s (frames=%d)."),
				*GetName(), EffectiveFlipbook ? *EffectiveFlipbook->GetName() : TEXT("NULL"), EffectiveFlipbook ? EffectiveFlipbook->GetNumKeyFrames() : 0);
		}
	}

	UPrimitiveComponent* VisualComponent = nullptr;
	if (FlipbookComponent && FlipbookComponent->IsVisible())
	{
		VisualComponent = FlipbookComponent.Get();
	}
	else if (SpriteComponent)
	{
		VisualComponent = SpriteComponent.Get();
	}

	if (VisualComponent)
	{
		BaseVisualScale3D = VisualComponent->GetRelativeScale3D();
		VisualComponent->SetRelativeScale3D(BaseVisualScale3D * RelativeScale3D);
		InitialVisualScaleX = FMath::Abs(VisualComponent->GetRelativeScale3D().X);

		float InitialVelocityX = 0.0f;
		if (bOneWayMovement)
		{
			InitialVelocityX = OneWaySpeed;
		}
		else if (bDynamic)
		{
			InitialVelocityX = SwaySpeed * FMath::Cos(SwayPhase);
		}
		UpdateVisualFacing(InitialVelocityX >= 0.0f);
	}

	if (bWanderInRadius)
	{
		const float Angle = FMath::RandRange(0.0f, 2.0f * PI);
		const float Radius = FMath::Sqrt(FMath::RandRange(0.0f, 1.0f)) * WanderRadius;
		WanderTarget = SpawnLocation + FVector(FMath::Cos(Angle) * Radius, 0.0f, FMath::Sin(Angle) * Radius);
		WanderTargetTimer = FMath::RandRange(0.5f, 1.5f);
	}

	UE_LOG(LogTemp, Log, TEXT("Obstacle %s spawned at %s (Dynamic=%d, OneWay=%d, Wander=%d, OneWaySpeed=%.1f, ScrollWithChunk=%d, SpriteCollision=%d, Effect=%s)."),
		*GetName(), *SpawnLocation.ToString(), bDynamic, bOneWayMovement, bWanderInRadius, OneWaySpeed, bScrollWithChunk, bUseSpriteCollision, *EffectTag.ToString());

	if (SpawnSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
	}

	if (SpawnEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SpawnEffect, GetActorLocation());
	}
}

void AObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedLifeTime += DeltaTime;
	CheckDestroyConditions();
	if (IsPendingKillPending())
	{
		return;
	}

	FVector Location = GetActorLocation();
	const float OldX = Location.X;

	if (bScrollWithChunk)
	{
		Location.Z += ScrollSpeed * DeltaTime;
	}

	if (bOneWayMovement)
	{
		Location.X += OneWaySpeed * DeltaTime;
	}
	else if (bWanderInRadius)
	{
		const float CenterZ = SpawnLocation.Z + (bScrollWithChunk ? ScrollSpeed * ElapsedLifeTime : 0.0f);
		const FVector Center = FVector(InitialX, Location.Y, CenterZ);

		WanderTargetTimer -= DeltaTime;
		const FVector ToTarget = WanderTarget - Location;
		const float DistToTargetSq = FVector2D(ToTarget.X, ToTarget.Z).SizeSquared();
		if (WanderTargetTimer <= 0.0f || DistToTargetSq < FMath::Square(10.0f))
		{
			const float Angle = FMath::RandRange(0.0f, 2.0f * PI);
			const float Radius = FMath::Sqrt(FMath::RandRange(0.0f, 1.0f)) * WanderRadius;
			WanderTarget = Center + FVector(FMath::Cos(Angle) * Radius, 0.0f, FMath::Sin(Angle) * Radius);
			WanderTargetTimer = FMath::RandRange(0.5f, 1.5f);
		}

		const FVector Direction = (WanderTarget - Location).GetSafeNormal();
		Location += Direction * WanderSpeed * DeltaTime;

		FVector Offset = Location - Center;
		Offset.Y = 0.0f;
		if (Offset.Size2D() > WanderRadius)
		{
			const FVector ClampedOffset = Offset.GetClampedToMaxSize2D(WanderRadius);
			Location.X = Center.X + ClampedOffset.X;
			Location.Z = Center.Z + ClampedOffset.Z;
		}
	}
	else if (bDynamic)
	{
		const float AngularSpeed = SwayAmplitude > KINDA_SMALL_NUMBER ? SwaySpeed / SwayAmplitude : 0.0f;
		SwayPhase += AngularSpeed * DeltaTime;
		Location.X = InitialX + FMath::Sin(SwayPhase) * SwayAmplitude;
	}

	const float DeltaX = Location.X - OldX;
	if (FMath::Abs(DeltaX) > KINDA_SMALL_NUMBER)
	{
		UpdateVisualFacing(DeltaX > 0.0f);
	}

	SetActorLocation(Location);
}

void AObstacle::CheckDestroyConditions()
{
	if (MaxLifeTime > 0.0f && ElapsedLifeTime >= MaxLifeTime)
	{
		Destroy();
		return;
	}

	if (MaxDistanceFromSpawn > 0.0f && FVector::Dist(GetActorLocation(), SpawnLocation) >= MaxDistanceFromSpawn)
	{
		Destroy();
		return;
	}
}

void AObstacle::OnHitPlayer(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsPlayerActor(OtherActor))
	{
		return;
	}

	if (bBreakableByDash && IsPlayerDashing(OtherActor))
	{
		OnDashBroken();
		return;
	}

	if (bApplyEffectOnce && bEffectAlreadyApplied)
	{
		return;
	}

	bEffectAlreadyApplied = true;

	if (AGameJamAnchorGameMode* GameMode = Cast<AGameJamAnchorGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->ReportPlayerHitObstacle(this, EffectTag);
	}

	ReceiveOnHitPlayer();
	PlayHitEffects();

	if (bDestroyOnHit)
	{
		PlayDestroyEffects();
		ReceiveOnDestroyed();
		Destroy();
	}
}

void AObstacle::OnDashBroken()
{
	UE_LOG(LogTemp, Log, TEXT("Obstacle %s broken by dash."), *GetName());
	ReceiveOnDashBroken();

	if (bDestroyOnDashBreak)
	{
		PlayDestroyEffects();
		ReceiveOnDestroyed();
		Destroy();
	}
	else
	{
		if (CollisionBox)
		{
			CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (SpriteComponent)
		{
			SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void AObstacle::PlayHitEffects()
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	if (HitEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitEffect, GetActorLocation());
	}
}

void AObstacle::PlayDestroyEffects()
{
	if (DestroySound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, DestroySound, GetActorLocation());
	}

	if (DestroyEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), DestroyEffect, GetActorLocation());
	}
}

bool AObstacle::IsPlayerDashing(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	if (Actor->GetClass()->ImplementsInterface(UAnchorPlayerInterface::StaticClass()))
	{
		return IAnchorPlayerInterface::Execute_IsDashing(Actor);
	}

	return false;
}

bool AObstacle::IsPlayerActor(AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	return Actor->ActorHasTag(FName(TEXT("Anchor.Player")));
}

void AObstacle::UpdateVisualFacing(bool bMovingRight)
{
	UPrimitiveComponent* VisualComponent = nullptr;
	if (FlipbookComponent && FlipbookComponent->IsVisible())
	{
		VisualComponent = FlipbookComponent.Get();
	}
	else if (SpriteComponent)
	{
		VisualComponent = SpriteComponent.Get();
	}

	if (!VisualComponent)
	{
		return;
	}

	const bool bShouldBeFlipped = bMovingRight != bFaceRightByDefault;
	FVector Scale = BaseVisualScale3D * RelativeScale3D;
	Scale.X = bShouldBeFlipped ? -InitialVisualScaleX : InitialVisualScaleX;
	if (bMirrorX)
	{
		Scale.X *= -1.0f;
	}
	VisualComponent->SetRelativeScale3D(Scale);
}

void AObstacle::ApplyVisualConfig()
{
	UPrimitiveComponent* VisualComponent = nullptr;
	if (FlipbookComponent && FlipbookComponent->IsVisible())
	{
		VisualComponent = FlipbookComponent.Get();
	}
	else if (SpriteComponent)
	{
		VisualComponent = SpriteComponent.Get();
	}

	if (!VisualComponent)
	{
		return;
	}

	VisualComponent->SetRelativeScale3D(BaseVisualScale3D * RelativeScale3D);
	InitialVisualScaleX = FMath::Abs(VisualComponent->GetRelativeScale3D().X);

	float VelocityX = 0.0f;
	if (bOneWayMovement)
	{
		VelocityX = OneWaySpeed;
	}
	else if (bDynamic)
	{
		VelocityX = SwaySpeed * FMath::Cos(SwayPhase);
	}
	UpdateVisualFacing(VelocityX >= 0.0f);

	UE_LOG(LogTemp, Log, TEXT("Obstacle %s ApplyVisualConfig: BaseScale=%s, RelativeScale=%s, Mirror=%d, FaceRight=%d, InitialScaleX=%.2f."),
		*GetName(), *BaseVisualScale3D.ToString(), *RelativeScale3D.ToString(), bMirrorX, bFaceRightByDefault, InitialVisualScaleX);
}
