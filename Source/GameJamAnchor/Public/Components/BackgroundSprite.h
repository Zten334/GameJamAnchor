// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperSpriteComponent.h"
#include "BackgroundSprite.generated.h"

class AAnchor;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEJAMANCHOR_API UBackgroundSprite : public UPaperSpriteComponent
{
	GENERATED_BODY()

public:
	UBackgroundSprite();

protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	float OffsetValue;
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> OwningPlayer;
	UPROPERTY(BlueprintReadOnly)
	float OffsetY;
	
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected:
	UFUNCTION(BlueprintCallable)
	void MatchOffset();

};


	
