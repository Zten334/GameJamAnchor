// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BackgroundSprite.h"

#include "Anchor.h"
#include "Kismet/GameplayStatics.h"


UBackgroundSprite::UBackgroundSprite()
{

	PrimaryComponentTick.bCanEverTick = false;

}

void UBackgroundSprite::BeginPlay()
{
	Super::BeginPlay();
	OwningPlayer = Cast<AAnchor>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (!OwningPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("No player pawn"));
	}
}



void UBackgroundSprite::TickComponent(float DeltaTime, ELevelTick TickType,
									 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	

	// GetPlayerLocation
	
}

