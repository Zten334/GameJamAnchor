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

	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,Category="Movement")
	float MaxSpeed;
#pragma region InputResponse
	void DoMove(const FInputActionValue& InputActionValue);
	void DoSprint(const FInputActionValue& InputActionValue);

#pragma endregion
	
	UFUNCTION(BlueprintCallable)
	void OnHit();
	
	UFUNCTION(BlueprintCallable)
	void OnWind(FVector2D Direction);
	
	UFUNCTION(BlueprintCallable)
	void OnTwine(float QUEValue);
private:
	float QTETime;

};
