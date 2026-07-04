// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "EventManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPressA);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPressB);
/**
 * 
 */
UCLASS()
class GAMEJAMANCHOR_API AEventManager : public AGameStateBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnPressA OnPressA;
	
	UPROPERTY(BlueprintAssignable,BlueprintCallable)
	FOnPressB OnPressB;

	
};
