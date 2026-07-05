// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Player/AnchorPlayerInterface.h"
#include "PaperSpriteComponent.h"
#include "AnchorPlayerPawn.generated.h"

class UMovementComponent;
class UCapsuleComponent;
/**
 * 玩家锚的占位 Pawn。
 * 另一位程序将接管真正的玩家移动、属性与失败判定。
 * 当前仅用于承载 Paper2D Sprite，方便在关卡中占位和测试相机。
 */
class UInputComponent;
struct FInputActionValue;
class UInputData;
class USpringArmComponent;
class UCameraComponent;

UCLASS(Blueprintable)
class GAMEJAMANCHOR_API AAnchorPlayerPawn : public ACharacter, public IAnchorPlayerInterface
{
	GENERATED_BODY()

public:
	AAnchorPlayerPawn(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditDefaultsOnly,Category="MovingItems")
	TSubclassOf<AActor> MovingItemType;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	
	//UPROPERTY(BlueprintReadOnly, Category = "MovingItems",meta = (AllowPrivateAccess = true))
	//TArray<TObjectPtr<TSubclassOf<AActor>> MovingItems;


protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Input")
	TObjectPtr<UInputData> InputData;

	/** 方向指示精灵，根据速度方向旋转。 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Anchor|Indicator")
	TObjectPtr<UPaperSpriteComponent> DirectionIndicator;

	/** 方向指示最小旋转角度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor|Indicator")
	float IndicatorMinAngle = -90.0f;

	/** 方向指示最大旋转角度。 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor|Indicator")
	float IndicatorMaxAngle = 90.0f;

	void UpdateDirectionIndicator();


	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="NormalSpeed")
	float MaxSpeed;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Sprint")
	float SprintSpeed;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Sprint")
	float SprintDurationTime;
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Sprint")
	float SprintCoolDownTime;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Swing")
	float SwingSpeed = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Swing")
	float MaxSwingAngle = 45.0f;

	
#pragma region InputResponse
	void DoMove(const FInputActionValue& InputActionValue);
	void DoSprint(const FInputActionValue& InputActionValue);
	void DoSprintStarted(const FInputActionValue& InputActionValue);
	void DoSprintOnGoing(const FInputActionValue& InputActionValue);
	void DoStruggle(const FInputActionValue& InputActionValue);
	void OnBreakAway();
	void EndSprint();
	void ResetSprint();
//暂时不需要
	void DoAcceleration(const FInputActionValue& InputActionValue);
	void DoDecelation(const FInputActionValue& InputActionValue);

#pragma endregion
	
#pragma region VelocityResponse	
	
	UPROPERTY(BluePrintReadOnly,EditAnywhere)
	FVector CurrentSprintDirection = FVector(0.0f, 0.0f, -1.0f);
	
	bool bIsAiming = false;
	
	UFUNCTION(BlueprintCallable)
	void OnHit();
	
	UFUNCTION(BlueprintCallable)
	void OnDeceleration(const float TimeValue,const float DecelerationRateValue);
	
	void EndDeceleration();
	
	UFUNCTION(BlueprintCallable)
	void OnHang();
	
	UFUNCTION(BlueprintCallable)
	void OnWind(FVector2D Direction);
	
	UFUNCTION(BlueprintCallable)
	void OnTwine(float QUEValue,AActor* TwinActor);

	// ── IAnchorPlayerInterface 实现 ──

	virtual bool IsDashing_Implementation() const override;
	virtual FVector GetDashDirection_Implementation() const override;
	virtual void ApplyCurrentForce_Implementation(FVector ForcePerSecond) override;

	// ── 障碍碰撞事件绑定与路由 ──

	UFUNCTION()
	void OnObstacleHitPlayer(AActor* Hitter, FName EffectTag);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|EffectTags")
	FName SlowEffectTag = "Slow";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|EffectTags")
	FName WindEffectTag = "Wind";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|EffectTags")
	FName HangEffectTag = "Hang";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|EffectTags")
	FName TwineEffectTag = "Twine";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|EffectTags")
	FName HitEffectTag = "Hit";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Deceleration")
	float DecelerationDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Deceleration")
	float DecelerationRate = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Wind")
	float WindStrength = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Obstacle|Twine")
	float TwineDefaultQTETime = 3.0f;

#pragma endregion
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Anchor|QTE")
	float QTETime = 0.0f;

	/** 是否处于悬挂状态。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor|QTE")
	bool isHanging = false;

	/** 缠绕目标 Actor。 */
	UPROPERTY(BlueprintReadOnly, Category = "Anchor|QTE")
	TWeakObjectPtr<AActor> FollowTargetActor;
	
	UPROPERTY(BlueprintReadWrite, Category = "Anchor|QTE")
	bool canSprint ;
private:
	FVector VelocityBeforeHanged;
	bool isSprinting = false;
	
	
	float CurrentSwingAngle = 45.0f;
	float SwingDir = -1.0f;
	
	
	
};
