// Fill out your copyright notice in the Description page of Project Settings.


#include "GameJamAnchor/Public/Player/AnchorPlayerPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "GameJamAnchor/Public/Player/Input/InputData.h"

#include "Blueprint/UserWidget.h"


// Sets default values
AAnchorPlayerPawn::AAnchorPlayerPawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    //暂时放开，方便蓝图后续使用
	PrimaryActorTick.bCanEverTick = ture;
	
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
		EnhancedInput->BindAction(InputData->SprintAction, ETriggerEvent::Triggered,this, &AAnchorPlayerPawn::DoSprint);
	}	
	
}

void AAnchorPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// 开放鼠标光标
	//PC->SetShowMouseCursor(true);
	
}

void AAnchorPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (QTETime > 0.0f)
	{
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
		QTETime -= DeltaTime;
	}
	
}

void AAnchorPlayerPawn::DoMove(const FInputActionValue& InputActionValue)
{
	const FVector2D Movement = InputActionValue.Get<FVector2D>();
	
	//UE_LOG(LogTemp, Warning, TEXT("Moving!"));
	if (!Controller || Movement.IsNearlyZero())
	{
		return;
	}

	//const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

	if (Movement.Y != 0.f)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Climbing!"));
		const FVector ForwardDirection = FVector(0,0,1.0f);
		UE_LOG(LogTemp, Warning, TEXT("%s"),*ForwardDirection.ToString());
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
}

void AAnchorPlayerPawn::OnHit()
{
	int type = 0;
	switch (type)
	{
	case 0:
		
		break;
		default:
		break;
	}
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

void AAnchorPlayerPawn::OnTwine(float QUEValue)
{
	
}






