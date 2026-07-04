// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputData.generated.h"

class UInputAction;
class UInputMappingContext;
/**
 * 
 */
UCLASS()
class GAMEJAMANCHOR_API UInputData : public UDataAsset
{
	GENERATED_BODY()
	
public :
UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input|Actions")
	TObjectPtr<UInputAction> SprintAction;
};
