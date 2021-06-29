// Fill out your copyright notice in the Description page of Project Settings.


#include "ScorePickUpCPP.h"

// Sets default values
AScorePickUpCPP::AScorePickUpCPP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AScorePickUpCPP::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AScorePickUpCPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AScorePickUpCPP::GetBoardPieceCrushCheckHeight_Implementation()
{
	return GetActorLocation().Z;
}
