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

	if (currentState != BOARD_PIECE_STATE::IDLE) {
		TimeSinceMovementStarted += DeltaTime;
	}

	if(currentState == BOARD_PIECE_STATE::SPAWNING){
		_DoPieceMove(RootLocation, RootLocation + FVector(0.0f, 0.0f, BoardPieceSpawnHeight), BOARD_PIECE_STATE::IDLE, TimeToFinishSpawnMovement);
	}
	else if (currentState == BOARD_PIECE_STATE::SWAP_UNDER) {
		_DoPieceMove(SwapTargetLocation, RootLocation, BOARD_PIECE_STATE::IDLE, TimeToFinishSwapUnderMovement);
	}
	else if (currentState == BOARD_PIECE_STATE::SWAP_OVER_1) {
		_DoPieceMove(RootLocation + FVector(0.0f, 0.0f, BoardPieceSwapHeight), RootLocation, BOARD_PIECE_STATE::SWAP_OVER_2, TimeToFinishSwapOverStepMovement);
	}
	else if (currentState == BOARD_PIECE_STATE::SWAP_OVER_2) {
		_DoPieceMove(SwapTargetLocation + FVector(0.0f, 0.0f, BoardPieceSwapHeight), RootLocation + FVector(0.0f, 0.0f, BoardPieceSwapHeight), BOARD_PIECE_STATE::SWAP_OVER_3, TimeToFinishSwapOverStepMovement);
	}
	else if (currentState == BOARD_PIECE_STATE::SWAP_OVER_3) {
		_DoPieceMove(SwapTargetLocation, SwapTargetLocation + FVector(0.0f, 0.0f, BoardPieceSwapHeight), BOARD_PIECE_STATE::IDLE, TimeToFinishSwapOverStepMovement);
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

void ABoardPieceCPP::_DoPieceMove(FVector TargetLocation, FVector StartingLocation, BOARD_PIECE_STATE nextState, float TimeToDoMove) {
	float alpha = FMath::Clamp((TimeToDoMove - TimeSinceMovementStarted) / TimeToDoMove, 0.0f, 1.0f);

	SetActorLocation(FMath::Lerp(TargetLocation, StartingLocation, alpha));

	if (TimeSinceMovementStarted >= TimeToDoMove) {
		currentState = nextState;
		TimeSinceMovementStarted = 0.0f;
	}
}