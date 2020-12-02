// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardPieceCPP.h"
#include "BoardPieceHolderCPP.h"

// Sets default values
ABoardPieceCPP::ABoardPieceCPP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABoardPieceCPP::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABoardPieceCPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(IsMoving){
		TimeSinceMovementStarted += DeltaTime;
		
		float alpha = FMath::Clamp((TimeToFinishMovement - TimeSinceMovementStarted)/ TimeToFinishMovement, 0.0f, 1.0f);
		
		SetActorLocation(FMath::Lerp(RootLocation, RootLocation + FVector(0.0f, 0.0f, BoardPieceSpawnHeight), alpha));
		
		if(TimeSinceMovementStarted >= TimeToFinishMovement){
			IsMoving = false;
		}
	}
}

bool ABoardPieceCPP::IsSameType(ABoardPieceCPP* Other)
{
	return BoardPieceType == Other->BoardPieceType;
}

void ABoardPieceCPP::ServerDoSpawnMovement_Implementation()
{
	_DoSpawnMovement();
}

void ABoardPieceCPP::_DoSpawnMovement()
{
	if(IsMoving){
		return;
	}
	
	IsMoving = true;
	TimeSinceMovementStarted = 0.0f;
	RootLocation = GetActorLocation();
	SetActorLocation(RootLocation + FVector(0.0f, 0.0f, BoardPieceSpawnHeight));
}