// Copyright Epic Games, Inc. All Rights Reserved.

#include "Obstacle/CurrentJetObstacle.h"
#include "Components/BoxComponent.h"
#include "Player/AnchorPlayerInterface.h"
#include "Framework/GameJamAnchorGameMode.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Particles/ParticleSystem.h"

ACurrentJetObstacle::ACurrentJetObstacle(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bDynamic = false;
	bOneWayMovement = false;
	bWanderInRadius = false;
	bScrollWithChunk = true;

	CurrentTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CurrentTriggerBox"));
	CurrentTriggerBox->SetupAttachment(RootComponent);
	CurrentTriggerBox->SetCollisionProfileName(FName("OverlapAllDynamic"));
	CurrentTriggerBox->SetGenerateOverlapEvents(true);
	CurrentTriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// CurrentJetObstacle 使用自己的 CurrentTriggerBox 做 Overlap 检测，
	// 不需要父类的 OverlapBox，禁用以避免重复触发。
	if (OverlapBox)
	{
		OverlapBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	CurrentTriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACurrentJetObstacle::OnCurrentBeginOverlap);
	CurrentTriggerBox->OnComponentEndOverlap.AddDynamic(this, &ACurrentJetObstacle::OnCurrentEndOverlap);
}

void ACurrentJetObstacle::BeginPlay()
{
	Super::BeginPlay();

	CooldownTimer = JetStartupDelay;
	StopJet();
}

void ACurrentJetObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsJetting)
	{
		ApplyCurrentToOverlappingPlayers(DeltaTime);

		JetTimer -= DeltaTime;
		if (JetTimer <= 0.0f)
		{
			StopJet();
			CooldownTimer = JetCooldown;
		}
	}
	else
	{
		CooldownTimer -= DeltaTime;
		if (CooldownTimer <= 0.0f)
		{
			StartJet();
		}
	}
}

void ACurrentJetObstacle::StartJet()
{
	if (bIsJetting)
	{
		return;
	}

	bIsJetting = true;
	JetTimer = JetDuration;
	TaggedPlayersThisCycle.Empty();

	if (CurrentTriggerBox)
	{
		CurrentTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}

	if (JetStartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JetStartSound, GetActorLocation());
	}

	if (JetStartEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), JetStartEffect, GetActorLocation());
	}

	UE_LOG(LogTemp, Log, TEXT("CurrentJet %s started jetting for %.1fs."), *GetName(), JetDuration);
}

void ACurrentJetObstacle::StopJet()
{
	bIsJetting = false;
	AffectedPlayers.Empty();
	TaggedPlayersThisCycle.Empty();

	if (CurrentTriggerBox)
	{
		CurrentTriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (JetEndSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, JetEndSound, GetActorLocation());
	}

	if (JetEndEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), JetEndEffect, GetActorLocation());
	}

	UE_LOG(LogTemp, Log, TEXT("CurrentJet %s stopped jetting."), *GetName());
}

void ACurrentJetObstacle::ApplyCurrentToOverlappingPlayers(float DeltaTime)
{
	if (!CurrentTriggerBox)
	{
		return;
	}

	TArray<AActor*> OverlappingActors;
	CurrentTriggerBox->GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (IsPlayerActor(Actor) && !AffectedPlayers.Contains(Actor))
		{
			AffectedPlayers.Add(Actor);
		}
	}

	for (int32 i = AffectedPlayers.Num() - 1; i >= 0; --i)
	{
		AActor* PlayerActor = AffectedPlayers[i];
		if (!PlayerActor || !CurrentTriggerBox->IsOverlappingActor(PlayerActor))
		{
			AffectedPlayers.RemoveAt(i);
			continue;
		}

		ApplyCurrentToPlayer(PlayerActor);
	}
}

void ACurrentJetObstacle::ApplyCurrentToPlayer(AActor* PlayerActor)
{
	if (!PlayerActor)
	{
		return;
	}

	if (PlayerActor->GetClass()->ImplementsInterface(UAnchorPlayerInterface::StaticClass()))
	{
		IAnchorPlayerInterface::Execute_ApplyCurrentForce(PlayerActor, CurrentForce);
	}

	if (!TaggedPlayersThisCycle.Contains(PlayerActor))
	{
		TaggedPlayersThisCycle.Add(PlayerActor);

		if (AGameJamAnchorGameMode* GameMode = Cast<AGameJamAnchorGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->ReportPlayerHitObstacle(this, EffectTag);
		}

		ReceiveOnHitPlayer();
		PlayHitEffects();
	}
}

void ACurrentJetObstacle::OnCurrentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsPlayerActor(OtherActor))
	{
		return;
	}

	if (!AffectedPlayers.Contains(OtherActor))
	{
		AffectedPlayers.Add(OtherActor);
	}
}

void ACurrentJetObstacle::OnCurrentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AffectedPlayers.Remove(OtherActor);
}
