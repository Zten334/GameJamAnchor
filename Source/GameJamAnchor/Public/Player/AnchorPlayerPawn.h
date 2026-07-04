// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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
class GAMEJAMANCHOR_API AAnchorPlayerPawn : public ACharacter
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
	void OnDeceleration(const float TimeValue,const float DecelerationRate);
	
	void EndDeceleration();
	
	UFUNCTION(BlueprintCallable)
	void OnHang();
	
	UFUNCTION(BlueprintCallable)
	void OnWind(FVector2D Direction);
	
	UFUNCTION(BlueprintCallable)
	void OnTwine(float QUEValue,AActor* TwinActor);

#pragma endregion
public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly)
	float QTETime;
	
private:
	FVector VelocityBeforeHanged;
	bool isHanging;
	bool isSprinting;
	bool canSprint ;
	
	UPROPERTY()
	TObjectPtr<AActor> FollowTargetActor;
	
	float CurrentSwingAngle = 45.0f;
	float SwingDir = -1.0f;
	
	
	
};
