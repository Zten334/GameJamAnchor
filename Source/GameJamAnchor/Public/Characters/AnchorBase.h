// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"

#include "AnchorBase.generated.h"


class UInputComponent;
struct FInputActionValue;
class UInputData;
class USpringArmComponent;
class UCameraComponent;
UCLASS()
class GAMEJAMANCHOR_API AAnchorBase : public APaperZDCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAnchorBase();

protected:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly,Category="MovingItems")
	TSubclassOf<AActor> MovingItemType;
	
	//UPROPERTY(BlueprintReadOnly, Category = "MovingItems",meta = (AllowPrivateAccess = true))
	//TArray<TObjectPtr<TSubclassOf<AActor>> MovingItems;

protected:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Input")
	TObjectPtr<UInputData> InputData;


#pragma region InputResponse
	void DoMove(const FInputActionValue& InputActionValue);
	void DoSprint(const FInputActionValue& InputActionValue);

#pragma endregion
	
};
