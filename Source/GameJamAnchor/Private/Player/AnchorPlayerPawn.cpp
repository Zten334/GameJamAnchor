// Fill out your copyright notice in the Description page of Project Settings.


#include "GameJamAnchor/Public/Player/AnchorPlayerPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HLSLMathAliases.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TimerManager.h"
#include "GameJamAnchor/Public/Player/Input/InputData.h"
#include "Framework/GameJamAnchorGameMode.h"
#include "Obstacle/Obstacle.h"
#include "Components/BoxComponent.h"

#include "Blueprint/UserWidget.h"
#include "DataWrappers/ChaosVDParticleDataWrapper.h"


// Sets default values
AAnchorPlayerPawn::AAnchorPlayerPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    //暂时放开，方便蓝图后续使用
	PrimaryActorTick.bCanEverTick = true;
	
	GetMesh()->bReceivesDecals = false;
	
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	GetCharacterMovement() -> bOrientRotationToMovement = false;
	GetCharacterMovement() -> RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement() -> JumpZVelocity = 500.f;
	GetCharacterMovement() -> AirControl = 0.35f;
	GetCharacterMovement() -> MaxWalkSpeed = 500.f;
	GetCharacterMovement() -> MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement() -> BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement() -> BrakingDecelerationFalling = 1500.f;
	
	MaxSpeed = 60.f;
	SprintSpeed = 100.f;
	SprintCoolDownTime = 2.5f;
	SprintDurationTime = 0.5f;
	canSprint = true;

	DirectionIndicator = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("DirectionIndicator"));
	DirectionIndicator->SetupAttachment(RootComponent);
}

void AAnchorPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// ── 防护 ──
	if (!InputData) { return; }

	// ── 1. 获取 EnhancedInputComponent ──
	UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	// ── 2. 将 MappingContext 添加到 EnhancedInputLocalPlayerSubsystem ──
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (InputData->InputMappingContext)
			{
				// priority 0 = 默认优先级
				Subsystem->AddMappingContext(InputData->InputMappingContext, 0);
			}
		}
	}
	// ── 3. 绑定 InputAction 到回调函数 ──
	if (InputData->MoveAction)
	{
		// ETriggerEvent::Triggered 每帧触发（用于持续输入如摇杆/WASD）
		EnhancedInput->BindAction(InputData->MoveAction, ETriggerEvent::Triggered,this, &AAnchorPlayerPawn::DoMove);
	}

	if (InputData->SprintAction)
	{
		EnhancedInput->BindAction(InputData->SprintAction, ETriggerEvent::Started,this, &AAnchorPlayerPawn::DoSprintStarted);
		EnhancedInput->BindAction(InputData->SprintAction, ETriggerEvent::Ongoing,this, &AAnchorPlayerPawn::DoSprintOnGoing);
		EnhancedInput->BindAction(InputData->SprintAction, ETriggerEvent::Triggered,this, &AAnchorPlayerPawn::DoSprint);
	}	
	
	if (InputData->StruggleAction)
	{
		EnhancedInput->BindAction(InputData->StruggleAction, ETriggerEvent::Triggered,this, &AAnchorPlayerPawn::DoStruggle);
	}	
	
	if (InputData->BreakAwayAction)
	{
		EnhancedInput->BindAction(InputData->BreakAwayAction, ETriggerEvent::Triggered,this, &AAnchorPlayerPawn::OnBreakAway);
	}	
}

void AAnchorPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// ── 玩家 Tag，供 AObstacle::IsPlayerActor 识别 ──
	Tags.Add(FName(TEXT("Anchor.Player")));

	// ── 绑定障碍碰撞事件 ──
	if (AGameJamAnchorGameMode* GameMode = Cast<AGameJamAnchorGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OnPlayerHitObstacle.AddDynamic(this, &AAnchorPlayerPawn::OnObstacleHitPlayer);
		GameMode->OnAnchorPlayerReachedGoal.AddDynamic(this, &AAnchorPlayerPawn::OnRaceEnd);
	}
}

void AAnchorPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateDirectionIndicator();

	//UE_LOG(LogTemp, Warning, TEXT("%f"),GetVelocity().Length());
	if (isHanging)
	{
		GetCharacterMovement() -> MaxFlySpeed = 0.f;
	}
	else if (QTETime > 0.0f)
	{
		QTETime -= DeltaTime;
		if (!FollowTargetActor.IsValid())
		{
			return;
		}
		SetActorLocation(FVector(FollowTargetActor->GetActorLocation().X,GetActorLocation().Y,FollowTargetActor->GetActorLocation().Z));
		
		/*
		const float TwineForce = 0.5f;
		FVector FinalDirection = FVector(0,0,1);
		if (GetVelocity().X > 0.f)
		{
			FinalDirection.X = -1.0f;
		}
		else if (GetVelocity().X < 0.f)
		{
			FinalDirection.X = 1.0f;
		}
		AddMovementInput(FinalDirection, TwineForce);
		
		*/
	}
	
	else if (isSprinting)
	{
		FVector FinalDirection = FVector(CurrentSprintDirection.X,0,-CurrentSprintDirection.Z);
		const float SprintForce = 3.f; 
		AddMovementInput(FinalDirection, SprintForce);
	}
	
	else if (FMath::IsNearlyZero(GetVelocity().Z))
	{
		if (GetActorLocation().Z > 1.f)
		{
			AddMovementInput(FVector(0,0,-1),1);
		}
		else if(GetActorLocation().Z < -1.f)
		{
			AddMovementInput(FVector(0,0,1),1);
		}

		// 障碍阻挡由 AObstacle::Tick 的 Sweep 移动负责，玩家侧不再主动推开。
	}
}

void AAnchorPlayerPawn::DoMove(const FInputActionValue& InputActionValue)
{
	const FVector2D Movement = InputActionValue.Get<FVector2D>();
	
	//UE_LOG(LogTemp, Warning, TEXT("%s"),*Movement.ToString());
	if (!Controller || Movement.IsNearlyZero())
	{
		return;
	}

	//const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	if (Movement.Y != 0.f)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Climbing!"));
		const FVector ForwardDirection = FVector(0,0,1.0f);
		//UE_LOG(LogTemp, Warning, TEXT("%s"),*ForwardDirection.ToString());
		AddMovementInput(ForwardDirection, Movement.Y);
	}
	if (Movement.X != 0.f)
	{
		const FVector RightDirection = FVector(1.0f,0,0);
		AddMovementInput(RightDirection, Movement.X);
	}
}

void AAnchorPlayerPawn::DoSprint(const FInputActionValue& InputActionValue)
{
	
    //先弄个简单的，只要速度大于最大速度，就直接判定正在加速中
	if (!canSprint){
		return;
	}
	canSprint = false;
	isSprinting = true;
	UE_LOG(LogTemp, Warning, TEXT("开始冲刺"));
	
	
	GetCharacterMovement()->MaxFlySpeed = SprintSpeed;
	//FVector SprintDirection = GetVelocity().IsNearlyZero() ? FVector::DownVector : FVector(0,0, abs(GetVelocity().GetSafeNormal().Z));
	
	//LaunchCharacter(SprintDirection * SprintSpeed, false, false);
	
	//GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	FTimerHandle SprintEndHandle;
	FTimerHandle SprintResetHandle;
	
	// 6. 【新增】设置冲刺持续时间计时器（时间到后恢复速度上限）
	GetWorldTimerManager().SetTimer(
		SprintEndHandle,
		this,
		&AAnchorPlayerPawn::EndSprint,
		SprintDurationTime,
		false
	);
	
	GetWorldTimerManager().SetTimer(
		SprintResetHandle,
		this,
		&AAnchorPlayerPawn::ResetSprint,
		SprintCoolDownTime,
		false
);

	
}

void AAnchorPlayerPawn::DoSprintStarted(const FInputActionValue& InputActionValue)
{
	bIsAiming = true;
	CurrentSwingAngle = 45.0f;
	SwingDir = -1.0f;
}

void AAnchorPlayerPawn::DoSprintOnGoing(const FInputActionValue& InputActionValue)
{
	if (!bIsAiming) { return; }

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	CurrentSwingAngle += SwingDir * SwingSpeed * DeltaTime;

	if (CurrentSwingAngle >= MaxSwingAngle)
	{
		CurrentSwingAngle = MaxSwingAngle;
		SwingDir = -1.0f;
	}
	else if (CurrentSwingAngle <= -MaxSwingAngle)
	{
		CurrentSwingAngle = -MaxSwingAngle;
		SwingDir = 1.0f;
	}

	FVector BaseDir(0.0f, 0.0f, 1.0f);
	CurrentSprintDirection = BaseDir.RotateAngleAxis(CurrentSwingAngle, FVector(0.0f, 1.0f, 0.0f));
	UE_LOG(LogTemp, Warning, TEXT("%s"),*CurrentSprintDirection.ToString());
	UE_LOG(LogTemp, Warning, TEXT("%f"),CurrentSwingAngle);
}

void AAnchorPlayerPawn::DoStruggle(const FInputActionValue& InputActionValue)
{
	//减少QTE时间
	QTETime -= 0.3f;
	QTETime =  FMath::Max(0.0f, QTETime);
}

void AAnchorPlayerPawn::OnBreakAway()
{
	isHanging = false;
	GetCharacterMovement()->MaxFlySpeed = MaxSpeed;
	GetCharacterMovement()->Velocity = FVector(-VelocityBeforeHanged.X,0,VelocityBeforeHanged.Z);
}

void AAnchorPlayerPawn::EndSprint()
{
	UE_LOG(LogTemp, Warning, TEXT("停止冲刺,%s"),*GetActorLocation().ToString());
	GetCharacterMovement()->MaxFlySpeed = 60.f;
	isSprinting = false;
}

void AAnchorPlayerPawn::ResetSprint()
{
	UE_LOG(LogTemp, Warning, TEXT("可以再次冲刺,%s"),*GetActorLocation().ToString());
	canSprint = true;
}

void AAnchorPlayerPawn::DoAcceleration(const FInputActionValue& InputActionValue)
{
}

void AAnchorPlayerPawn::DoDecelation(const FInputActionValue& InputActionValue)
{
}

void AAnchorPlayerPawn::OnHit()
{
	UE_LOG(LogTemp, Log, TEXT("Pawn: OnHit - creating game-over UI."));

	if (!LoseScreenWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pawn: LoseScreenWidgetClass is not set. Cannot show game-over screen."));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	UUserWidget* LoseWidget = CreateWidget<UUserWidget>(PC, LoseScreenWidgetClass);
	if (LoseWidget)
	{
		LoseWidget->AddToViewport();

		// 显示鼠标，允许玩家点击 UI 按钮
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());

		UE_LOG(LogTemp, Log, TEXT("Pawn: Game-over UI added to viewport."));
	}
}

void AAnchorPlayerPawn::OnRaceEnd()
{
	UE_LOG(LogTemp, Log, TEXT("Pawn: OnRaceEnd - creating victory UI."));

	if (!VictoryScreenWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pawn: VictoryScreenWidgetClass is not set. Cannot show victory screen."));
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC)
	{
		return;
	}

	UUserWidget* VictoryWidget = CreateWidget<UUserWidget>(PC, VictoryScreenWidgetClass);
	if (VictoryWidget)
	{
		VictoryWidget->AddToViewport();

		// 显示鼠标，允许玩家点击 UI 按钮
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeUIOnly());

		UE_LOG(LogTemp, Log, TEXT("Pawn: Victory UI added to viewport."));
	}
}

void AAnchorPlayerPawn::OnDeceleration(const float TimeValue,const float DecelerationRateValue)
{
	GetCharacterMovement()->MaxFlySpeed = MaxSpeed * DecelerationRateValue;
	FTimerHandle DecelerationEndHandle;
	
	// 6. 【新增】设置冲刺持续时间计时器（时间到后恢复速度上限）
	GetWorldTimerManager().SetTimer(
		DecelerationEndHandle,
		this,
		&AAnchorPlayerPawn::EndDeceleration,
		TimeValue,
		false
	);
	
}
//只在小于MaxSpeed的时候设置即可，允许玩家冲刺抵消
void AAnchorPlayerPawn::EndDeceleration()
{
	if (GetVelocity().Length() < MaxSpeed)
	{
		GetCharacterMovement()->MaxFlySpeed = MaxSpeed;
	}
}

void AAnchorPlayerPawn::OnHang()
{
	isHanging = true;
}

void AAnchorPlayerPawn::OnWind(FVector2D Direction)
{
	if (Direction.IsNearlyZero())
	{
		return;
	}
	const float WindForce = 0.5f; 
	const FVector FinalDirection = FVector(Direction.X,0,Direction.Y);
	AddMovementInput(FinalDirection, WindForce);
}

void AAnchorPlayerPawn::OnTwine(float QUEValue,AActor* TwinActor)
{
	FollowTargetActor = TwinActor;
	QTETime += QUEValue;
}

// ── 障碍 EffectTag 路由 ──

void AAnchorPlayerPawn::OnObstacleHitPlayer(AActor* Hitter, FName EffectTag)
{
	if (!Hitter)
	{
		return;
	}

	if (EffectTag == SlowEffectTag)
	{
		OnDeceleration(DecelerationDuration, DecelerationRate);
		UE_LOG(LogTemp, Log, TEXT("Pawn: Obstacle [%s] triggered Slow."), *Hitter->GetName());
	}
	else if (EffectTag == WindEffectTag)
	{
		const FVector Dir = (GetActorLocation() - Hitter->GetActorLocation()).GetSafeNormal2D();
		OnWind(FVector2D(Dir.X, Dir.Z) * WindStrength);
		UE_LOG(LogTemp, Log, TEXT("Pawn: Obstacle [%s] triggered Wind."), *Hitter->GetName());
	}
	else if (EffectTag == HangEffectTag)
	{
		OnHang();
		UE_LOG(LogTemp, Log, TEXT("Pawn: Obstacle [%s] triggered Hang."), *Hitter->GetName());
	}
	else if (EffectTag == TwineEffectTag)
	{
		OnTwine(TwineDefaultQTETime, Hitter);
		UE_LOG(LogTemp, Log, TEXT("Pawn: Obstacle [%s] triggered Twine."), *Hitter->GetName());
	}
	else if (EffectTag == HitEffectTag)
	{
		OnHit();
		UE_LOG(LogTemp, Log, TEXT("Pawn: Obstacle [%s] triggered Hit (Game Over)."), *Hitter->GetName());
	}
	else if (EffectTag == VictoryEffectTag)
	{
		OnRaceEnd();
		UE_LOG(LogTemp, Log, TEXT("Pawn: Obstacle [%s] triggered Victory."), *Hitter->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Pawn: Unknown EffectTag [%s] from Obstacle [%s]."), *EffectTag.ToString(), *Hitter->GetName());
	}
}

// ── IAnchorPlayerInterface 实现 ──

bool AAnchorPlayerPawn::IsDashing_Implementation() const
{
	return isSprinting;
}

FVector AAnchorPlayerPawn::GetDashDirection_Implementation() const
{
	return CurrentSprintDirection;
}

void AAnchorPlayerPawn::ApplyCurrentForce_Implementation(FVector ForcePerSecond)
{
	AddMovementInput(ForcePerSecond.GetSafeNormal(), ForcePerSecond.Size() * 0.01f);
}

// ── 方向指示 ──

void AAnchorPlayerPawn::UpdateDirectionIndicator()
{
	if (!DirectionIndicator)
	{
		return;
	}

	const FVector Vel = GetVelocity();
	if (Vel.IsNearlyZero())
	{
		return;
	}

	// 速度在 XZ 平面的方向角
	const float Angle = FMath::RadiansToDegrees(FMath::Atan2(Vel.X, Vel.Z));

	// 钳制到配置的角度范围
	const float ClampedAngle = FMath::Clamp(Angle, IndicatorMinAngle, IndicatorMaxAngle);

	DirectionIndicator->SetRelativeRotation(FRotator(ClampedAngle, 0.0f, 0.0f));
}



