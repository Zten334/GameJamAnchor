// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BackgroundSprite.h"

#include "Anchor.h"
#include "Kismet/GameplayStatics.h"


UBackgroundSprite::UBackgroundSprite()
{

	PrimaryComponentTick.bCanEverTick = true;
	OffsetValue = 6.f;

}

void UBackgroundSprite::BeginPlay()
{
	Super::BeginPlay();
	OwningPlayer = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!OwningPlayer)
	{	
		UE_LOG(LogTemp, Error, TEXT("No player pawn"));
	}
	else
	{
		OffsetY = 1.0f/abs( OwningPlayer->GetActorLocation().Y - GetComponentLocation().Y);
	}
}



void UBackgroundSprite::TickComponent(float DeltaTime, ELevelTick TickType,
									 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	
	MatchOffset();
}

void UBackgroundSprite::MatchOffset()
{
	float SpeedZ = OwningPlayer->GetVelocity().Z;
	FVector FinalLocation = GetComponentLocation();
	FinalLocation.Z += SpeedZ * OffsetValue * OffsetY; 
	SetWorldLocation(FinalLocation);
}


