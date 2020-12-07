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

	float swapTimeToFinishMovement = TimeToFinishMovement / 3.0f;

	if(currentState == BOARD_PIECE_STATE::SPAWNING){
		TimeSinceMovementStarted += DeltaTime;
		
		float alpha = FMath::Clamp((TimeToFinishMovement - TimeSinceMovementStarted)/ TimeToFinishMovement, 0.0f, 1.0f);
		
		SetActorLocation(FMath::Lerp(RootLocation, RootLocation + FVector(0.0f, 0.0f, BoardPieceSpawnHeight), alpha));
		
		if(TimeSinceMovementStarted >= TimeToFinishMovement){
			currentState = BOARD_PIECE_STATE::IDLE;
		}
	}
	else if (currentState == BOARD_PIECE_STATE::SWAP_UNDER) {
		TimeSinceMovementStarted += DeltaTime;

		float alpha = FMath::Clamp((swapTimeToFinishMovement - TimeSinceMovementStarted) / swapTimeToFinishMovement, 0.0f, 1.0f);

		SetActorLocation(FMath::Lerp(SwapTargetLocation, RootLocation, alpha));

		if (TimeSinceMovementStarted >= swapTimeToFinishMovement) {
			currentState = BOARD_PIECE_STATE::IDLE;
		}
	}
	else if (currentState == BOARD_PIECE_STATE::SWAP_OVER_1) {
		TimeSinceMovementStarted += DeltaTime;

		float alpha = FMath::Clamp((swapTimeToFinishMovement - TimeSinceMovementStarted) / swapTimeToFinishMovement, 0.0f, 1.0f);

		SetActorLocation(FMath::Lerp(RootLocation + FVector(0.0f, 0.0f, BoardPieceSwapHeight), RootLocation, alpha));

		if (TimeSinceMovementStarted >= swapTimeToFinishMovement) {
			currentState = BOARD_PIECE_STATE::SWAP_OVER_2;
			TimeSinceMovementStarted = 0.0f;
		}
	}
	else if (currentState == BOARD_PIECE_STATE::SWAP_OVER_2) {
		TimeSinceMovementStarted += DeltaTime;

		float alpha = FMath::Clamp((swapTimeToFinishMovement - TimeSinceMovementStarted) / swapTimeToFinishMovement, 0.0f, 1.0f);

		SetActorLocation(FMath::Lerp(SwapTargetLocation + FVector(0.0f, 0.0f, BoardPieceSwapHeight), RootLocation + FVector(0.0f, 0.0f, BoardPieceSwapHeight), alpha));

		if (TimeSinceMovementStarted >= swapTimeToFinishMovement) {
			currentState = BOARD_PIECE_STATE::SWAP_OVER_3;
			TimeSinceMovementStarted = 0.0f;
		}
	}
	else if (currentState == BOARD_PIECE_STATE::SWAP_OVER_3) {
		TimeSinceMovementStarted += DeltaTime;

		float alpha = FMath::Clamp((swapTimeToFinishMovement - TimeSinceMovementStarted) / swapTimeToFinishMovement, 0.0f, 1.0f);

		SetActorLocation(FMath::Lerp(SwapTargetLocation, SwapTargetLocation + FVector(0.0f, 0.0f, BoardPieceSwapHeight), alpha));

		if (TimeSinceMovementStarted >= swapTimeToFinishMovement) {
			currentState = BOARD_PIECE_STATE::IDLE;
			TimeSinceMovementStarted = 0.0f;
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
	if(currentState != BOARD_PIECE_STATE::IDLE){
		return;
	}
	
	currentState = BOARD_PIECE_STATE::SPAWNING;
	TimeSinceMovementStarted = 0.0f;
	RootLocation = GetActorLocation();
	SetActorLocation(RootLocation + FVector(0.0f, 0.0f, BoardPieceSpawnHeight));
}

void ABoardPieceCPP::ServerDoSwapMovement_Implementation(FVector TargetLocation, bool isMovingUnder)
{
	_DoSwapMovement(TargetLocation, isMovingUnder);
}

void ABoardPieceCPP::_DoSwapMovement(FVector TargetLocation, bool isMovingUnder)
{
	if (currentState != BOARD_PIECE_STATE::IDLE) {
		return;
	}

	if (isMovingUnder) {
		currentState = BOARD_PIECE_STATE::SWAP_UNDER;
	}
	else {
		currentState = BOARD_PIECE_STATE::SWAP_OVER_1;
	}

	TimeSinceMovementStarted = 0.0f;
	RootLocation = GetActorLocation();
	SwapTargetLocation = TargetLocation;
}

bool ABoardPieceCPP::IsPieceMoving(){
	return currentState != BOARD_PIECE_STATE::IDLE;
}