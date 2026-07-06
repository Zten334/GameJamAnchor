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
#include "Framework/SoundManager.h"
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
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Ignore);
	CollisionBox->ShapeColor = FColor::Red;

	OverlapBox = CreateDefaultSubobject<UBoxComponent>(TEXT("OverlapBox"));
	OverlapBox->SetupAttachment(RootComponent);
	OverlapBox->SetCollisionProfileName(FName("OverlapAllDynamic"));
	OverlapBox->SetGenerateOverlapEvents(true);
	OverlapBox->ShapeColor = FColor::Blue;

	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootScene);

	FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("FlipbookComponent"));
	FlipbookComponent->SetupAttachment(RootScene);
	FlipbookComponent->SetVisibility(false);
	FlipbookComponent->SetHiddenInGame(true);

	WarningSpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("WarningSpriteComponent"));
	WarningSpriteComponent->SetupAttachment(RootScene);
	WarningSpriteComponent->SetVisibility(false);
	WarningSpriteComponent->SetHiddenInGame(true);
	WarningSpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (UPaperSprite* DefaultSprite = LoadObject<UPaperSprite>(nullptr, TEXT("/Engine/EditorResources/S_Actor")))
	{
		SpriteComponent->SetSprite(DefaultSprite);
	}
}

UBoxComponent* AObstacle::GetCollisionBox() const
{
	return CollisionBox.Get();
}

void AObstacle::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!OverlapBox)
	{
		return;
	}

	// 方案 A：基于 Sprite/Flipbook 的实际渲染尺寸，加上 Padding
	// 计算有效缩放 = 蓝图编辑器缩放 × RelativeScale3D（BeginPlay 中会叠加）
	FVector BaseExtent = FVector(50.0f, 10.0f, 50.0f); // 默认兜底

	auto CalcSpriteExtent = [](UPaperSprite* Sprite, const FVector& EffectiveScale) -> FVector
	{
		FVector2D SourceSize = FVector2D::ZeroVector;
		if (UTexture2D* BakedTexture = Sprite->GetBakedTexture())
		{
			SourceSize = FVector2D(BakedTexture->GetSizeX(), BakedTexture->GetSizeY());
		}
		const float PPU = Sprite->GetPixelsPerUnrealUnit();
		const float WorldX = (SourceSize.X / PPU) * 0.5f * FMath::Abs(EffectiveScale.X);
		const float WorldZ = (SourceSize.Y / PPU) * 0.5f * FMath::Abs(EffectiveScale.Z);
		return FVector(WorldX, 10.0f, WorldZ);
	};

	// 优先用 Flipbook 的尺寸
	if (Flipbook.Get() && Flipbook->GetNumKeyFrames() > 0)
	{
		if (UPaperSprite* FirstSprite = Flipbook->GetKeyFrameChecked(0).Sprite)
		{
			const FVector EffectiveScale = FlipbookComponent->GetComponentScale() * RelativeScale3D;
			BaseExtent = CalcSpriteExtent(FirstSprite, EffectiveScale);
		}
	}
	else if (SpriteComponent && SpriteComponent->GetSprite())
	{
		const FVector EffectiveScale = SpriteComponent->GetComponentScale() * RelativeScale3D;
		BaseExtent = CalcSpriteExtent(SpriteComponent->GetSprite(), EffectiveScale);
	}

	OverlapBox->SetBoxExtent(BaseExtent + OverlapBoxPadding);
}

void AObstacle::BeginPlay()
{
	Super::BeginPlay();
	SpawnLocation = GetActorLocation();

	UE_LOG(LogTemp, Warning, TEXT("OBSTACLE_BEGINPLAY %s: Loc=%s SpawnLoc=%s bScrollWithChunk=%d"),
		*GetName(), *GetActorLocation().ToString(), *SpawnLocation.ToString(), (int32)bScrollWithChunk);

	// 记录蓝图里手动调整的 CollisionBox 尺寸，作为非 Sprite 碰撞模式下的缩放基准。
	if (CollisionBox)
	{
		BaseCollisionExtent = CollisionBox->GetUnscaledBoxExtent();
	}

	UPrimitiveComponent* HitComponent = CollisionBox.Get();

	// 强制关闭所有视觉/碰撞组件的物理模拟，防止蓝图中误开 Simulate Physics 导致障碍下落。
	auto DisablePhysics = [](UPrimitiveComponent* Comp)
	{
		if (!Comp)
		{
			return;
		}
		Comp->SetSimulatePhysics(false);
		Comp->SetEnableGravity(false);
	};
	DisablePhysics(CollisionBox.Get());
	DisablePhysics(OverlapBox.Get());
	DisablePhysics(SpriteComponent.Get());
	DisablePhysics(FlipbookComponent.Get());
	DisablePhysics(WarningSpriteComponent.Get());

	// 强制重置所有碰撞组件的 Profile，覆盖蓝图中可能保存的缺失/Custom profile。
	if (CollisionBox)
	{
		CollisionBox->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CollisionBox->SetGenerateOverlapEvents(true);
	}
	if (OverlapBox)
	{
		OverlapBox->SetCollisionProfileName(FName("OverlapAllDynamic"));
		OverlapBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		OverlapBox->SetGenerateOverlapEvents(true);
	}
	if (SpriteComponent)
	{
		if (bUseSpriteCollision)
		{
			SpriteComponent->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
			SpriteComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			SpriteComponent->SetGenerateOverlapEvents(true);
			HitComponent = SpriteComponent.Get();
		}
		else
		{
			SpriteComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
			SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SpriteComponent->SetGenerateOverlapEvents(false);
		}
	}
	if (WarningSpriteComponent)
	{
		WarningSpriteComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
		WarningSpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WarningSpriteComponent->SetGenerateOverlapEvents(false);
	}

	if (HitComponent)
	{
		// 不再绑定 OnComponentHit —— 障碍物通过 SetActorLocation 移动，
		// 玩家静止时 Hit 事件不会触发。改为使用 OverlapBox 检测。
	}

	// 不再绑定 Overlap 事件。
	// SetActorLocation（传送）移动方式下，物理系统 Overlap 事件不可靠。
	// 改为在 Tick 中通过 GetOverlappingActors 手动检测。
	OverlappedPlayers.Empty();

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

	UpdateWarningVisual();

	UE_LOG(LogTemp, Log, TEXT("Obstacle %s spawned at %s (Dynamic=%d, OneWay=%d, Wander=%d, OneWaySpeed=%.1f, ScrollWithChunk=%d, SpriteCollision=%d, Effect=%s)."),
		*GetName(), *SpawnLocation.ToString(), bDynamic, bOneWayMovement, bWanderInRadius, OneWaySpeed, bScrollWithChunk, bUseSpriteCollision, *EffectTag.ToString());

	if (SpawnSound)
	{
		if (ASoundManager* SM = Cast<ASoundManager>(GetWorld()->GetGameState()))
		{
			SM->PlaySoundAtLocation(SpawnSound, GetActorLocation());
		}
	}

	if (SpawnEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SpawnEffect, GetActorLocation());
	}
}

void AObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsGameOver())
	{
		return;
	}

	ElapsedLifeTime += DeltaTime;
	CheckDestroyConditions();
	if (IsPendingKillPending())
	{
		return;
	}

	const FVector OldLocation = GetActorLocation();
	FVector Location = OldLocation;
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

	// ── 手动 Sweep 移动 ──
	// SetActorLocation(..., true) 只在根组件带碰撞时才有效。障碍根组件是 SceneComponent，
	// 因此对实际生效的碰撞组件（CollisionBox 或 bUseSpriteCollision 时的 SpriteComponent）手动 Sweep。
	UPrimitiveComponent* BlockingComp = nullptr;
	if (CollisionBox && CollisionBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		BlockingComp = CollisionBox.Get();
	}
	else if (bUseSpriteCollision && SpriteComponent && SpriteComponent->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		BlockingComp = SpriteComponent.Get();
	}

	if (BlockingComp)
	{
		const FCollisionShape Shape = BlockingComp->GetCollisionShape();
		FCollisionQueryParams Params(NAME_None, false, this);
		FHitResult SweepHit;
		const bool bHit = GetWorld()->SweepSingleByProfile(
			SweepHit,
			OldLocation,
			Location,
			BlockingComp->GetComponentQuat(),
			BlockingComp->GetCollisionProfileName(),
			Shape,
			Params);

		if (bHit && SweepHit.IsValidBlockingHit())
		{
			Location = SweepHit.Location;

			AActor* HitActor = SweepHit.GetActor();
			if (IsPlayerActor(HitActor) && !OverlappedPlayers.Contains(HitActor))
			{
				OverlappedPlayers.Add(HitActor);
				OnOverlapBegin(OverlapBox.Get(), HitActor, nullptr, 0, true, SweepHit);
			}
		}
	}

	SetActorLocation(Location, false);

	// 若本次移动触发了游戏结束（死亡/胜利），立即回退本帧位移，确保场景彻底冻结。
	if (IsGameOver())
	{
		SetActorLocation(OldLocation, false);
		return;
	}

	// ── 手动 Overlap 检测 ──
	// 处理 Sweep 没拦到的低速/静止重叠情况（如玩家主动走进障碍）。
	// 开局 0.3 秒内跳过检测，防止出生时与玩家重叠误触发
	if (ElapsedLifeTime < 0.3f)
	{
		return;
	}

	if (OverlapBox && OverlapBox->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
	{
		TArray<AActor*> CurrentlyOverlapping;
		OverlapBox->GetOverlappingActors(CurrentlyOverlapping);

		// 找出新进入的玩家（Enter）—— 效果触发
		for (AActor* Actor : CurrentlyOverlapping)
		{
			if (IsPlayerActor(Actor) && !OverlappedPlayers.Contains(Actor))
			{
				OverlappedPlayers.Add(Actor);
				OnOverlapBegin(OverlapBox.Get(), Actor, nullptr, 0, false, FHitResult());
			}
		}

		// 找出发出离开的玩家（Leave）
		TArray<AActor*> ToRemove;
		for (AActor* Actor : OverlappedPlayers)
		{
			if (!CurrentlyOverlapping.Contains(Actor))
			{
				ToRemove.Add(Actor);
			}
		}
		for (AActor* Actor : ToRemove)
		{
			OverlappedPlayers.Remove(Actor);
			OnOverlapEnd(OverlapBox.Get(), Actor, nullptr, 0);
		}
	}
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
	if (IsGameOver() || !IsPlayerActor(OtherActor))
	{
		return;
	}

	if (bBreakableByDash && IsPlayerDashing(OtherActor))
	{
		OnDashBroken();
		return;
	}

	if (bApplyEffectOnce && AlreadyAffectedPlayers.Contains(OtherActor))
	{
		return;
	}

	AlreadyAffectedPlayers.Add(OtherActor);

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

void AObstacle::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsGameOver() || !IsPlayerActor(OtherActor))
	{
		return;
	}

	if (bBreakableByDash && IsPlayerDashing(OtherActor))
	{
		OnDashBroken();
		return;
	}

	if (bApplyEffectOnce && AlreadyAffectedPlayers.Contains(OtherActor))
	{
		return;
	}

	AlreadyAffectedPlayers.Add(OtherActor);

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

void AObstacle::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsPlayerActor(OtherActor))
	{
		OverlappedPlayers.Remove(OtherActor);
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

void AObstacle::Freeze()
{
	bScrollWithChunk = false;
	ScrollSpeed = 0.0f;
	bDynamic = false;
	bOneWayMovement = false;
	bWanderInRadius = false;
	OneWaySpeed = 0.0f;
	SwaySpeed = 0.0f;
	WanderSpeed = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Obstacle %s frozen."), *GetName());
}

void AObstacle::PlayHitEffects()
{
	if (HitSound)
	{
		if (ASoundManager* SM = Cast<ASoundManager>(GetWorld()->GetGameState()))
		{
			SM->PlaySoundAtLocation(HitSound, GetActorLocation());
		}
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
		if (ASoundManager* SM = Cast<ASoundManager>(GetWorld()->GetGameState()))
		{
			SM->PlaySoundAtLocation(DestroySound, GetActorLocation());
		}
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

bool AObstacle::IsGameOver() const
{
	if (const AGameJamAnchorGameMode* GameMode = Cast<AGameJamAnchorGameMode>(GetWorld()->GetAuthGameMode()))
	{
		return GameMode->IsGameOver();
	}
	return false;
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
	UpdateWarningVisual();

	// 视觉缩放变更后同步更新 CollisionBox 与 OverlapBox
	{
		FVector NewExtent(50.0f, 10.0f, 50.0f);
		UPaperSprite* RefSprite = nullptr;
		if (Flipbook.Get() && Flipbook->GetNumKeyFrames() > 0)
		{
			RefSprite = Flipbook->GetKeyFrameChecked(0).Sprite;
		}
		else if (SpriteComponent)
		{
			RefSprite = SpriteComponent->GetSprite();
		}

		if (RefSprite)
		{
			FVector2D SourceSize = FVector2D::ZeroVector;
			if (UTexture2D* BakedTexture = RefSprite->GetBakedTexture())
			{
				SourceSize = FVector2D(BakedTexture->GetSizeX(), BakedTexture->GetSizeY());
			}
			const float PPU = RefSprite->GetPixelsPerUnrealUnit();
			const FVector Scale = VisualComponent->GetComponentScale();
			const float WorldX = (SourceSize.X / PPU) * 0.5f * FMath::Abs(Scale.X);
			const float WorldZ = (SourceSize.Y / PPU) * 0.5f * FMath::Abs(Scale.Z);
			NewExtent = FVector(WorldX, 10.0f, WorldZ);
		}

		// 视觉缩放变更后同步更新 CollisionBox 与 OverlapBox。
		// - 使用 Sprite 碰撞时：CollisionBox 按 Sprite/Flipbook 尺寸生成，作为 Sweep 近似体。
		// - 不使用 Sprite 碰撞时：CollisionBox 保留蓝图手动调整的尺寸，并按 RelativeScale3D 缩放。
		if (CollisionBox)
		{
			if (bUseSpriteCollision)
			{
				CollisionBox->SetBoxExtent(NewExtent);
			}
			else
			{
				CollisionBox->SetBoxExtent(BaseCollisionExtent * RelativeScale3D);
			}
		}
		if (OverlapBox)
		{
			OverlapBox->SetBoxExtent(NewExtent + OverlapBoxPadding);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Obstacle %s ApplyVisualConfig: BaseScale=%s, RelativeScale=%s, Mirror=%d, FaceRight=%d, InitialScaleX=%.2f."),
		*GetName(), *BaseVisualScale3D.ToString(), *RelativeScale3D.ToString(), bMirrorX, bFaceRightByDefault, InitialVisualScaleX);
}

void AObstacle::UpdateWarningVisual()
{
	if (!WarningSpriteComponent)
	{
		return;
	}

	if (!bShowWarning || !WarningSprite)
	{
		WarningSpriteComponent->SetVisibility(false);
		WarningSpriteComponent->SetHiddenInGame(true);
		return;
	}

	WarningSpriteComponent->SetSprite(WarningSprite);
	WarningSpriteComponent->SetRelativeLocation(WarningOffset);
	WarningSpriteComponent->SetRelativeScale3D(FVector::OneVector * WarningSpriteScale);
	WarningSpriteComponent->SetVisibility(true);
	WarningSpriteComponent->SetHiddenInGame(false);
}
