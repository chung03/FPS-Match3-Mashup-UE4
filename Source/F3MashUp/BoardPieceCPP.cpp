// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardPieceCPP.h"
#include "BoardPieceHolderCPP.h"
#include "F3MashUpCharacter.h"

DEFINE_LOG_CATEGORY_STATIC(LogBoardPiece, Warning, All);

// Sets default values
ABoardPieceCPP::ABoardPieceCPP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SwapInitiatingPlayerId = -1;
}

// Called when the game starts or when spawned
void ABoardPieceCPP::BeginPlay()
{
	Super::BeginPlay();

	// Get all static mesh components
	TArray<UStaticMeshComponent*> StaticComps;
	GetComponents<UStaticMeshComponent>(StaticComps);

	for (UStaticMeshComponent* staticMesh : StaticComps)
	{
		_staticMesh = staticMesh;
	}
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
	else if (currentState == BOARD_PIECE_STATE::SWAP_UNDER_WAIT) {
		_DoPieceMove(RootLocation, RootLocation, BOARD_PIECE_STATE::SWAP_UNDER, TimeToFinishSwapUnderWait);
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

void ABoardPieceCPP::ServerDoSwapMovement_Implementation(FVector TargetLocation, bool isMovingUnder, int swappingPlayerId)
{
	_DoSwapMovement(TargetLocation, isMovingUnder, swappingPlayerId);
}

void ABoardPieceCPP::_DoSwapMovement(FVector TargetLocation, bool isMovingUnder, int swappingPlayerId)
{
	if (currentState != BOARD_PIECE_STATE::IDLE) {
		return;
	}

	if (isMovingUnder) {
		currentState = BOARD_PIECE_STATE::SWAP_UNDER_WAIT;
	}
	else {
		currentState = BOARD_PIECE_STATE::SWAP_OVER_1;
	}

	TimeSinceMovementStarted = 0.0f;
	RootLocation = GetActorLocation();
	SwapTargetLocation = TargetLocation;
	SwapInitiatingPlayerId = swappingPlayerId;
}

bool ABoardPieceCPP::IsPieceMoving(){
	return currentState != BOARD_PIECE_STATE::IDLE;
}

void ABoardPieceCPP::_DoPieceMove(FVector TargetLocation, FVector StartingLocation, BOARD_PIECE_STATE nextState, float TimeToDoMove) {
	float alpha = FMath::Clamp((TimeToDoMove - TimeSinceMovementStarted) / TimeToDoMove, 0.0f, 1.0f);

	// Doing the same move so that nothing really gets blocked
	SetActorLocation(FMath::Lerp(TargetLocation, StartingLocation, alpha));

	TArray<FHitResult> outHits;
	FVector traceStart = GetActorLocation();

	FVector traceEnd = GetActorLocation();

	FQuat rotation = FQuat().Identity;
	ECollisionChannel collisionChannel = ECollisionChannel::ECC_Pawn;
	
	FCollisionShape collisionShape = _staticMesh->GetCollisionShape();

	//Make the bounding box smaller so that it doesn't detect everything
	collisionShape.Box.HalfExtentX *= 0.98f;
	collisionShape.Box.HalfExtentY *= 0.98f;
	
	//Make box trace shorter and make it only check bottom of the board piece
	collisionShape.Box.HalfExtentZ = 5.0f;
	traceStart.Z -= collisionShape.Box.HalfExtentZ + 1.0f;
	traceEnd.Z -= collisionShape.Box.HalfExtentZ + 1.0f;

	/*
	const FName TraceTag("MyTraceTag");
	GetWorld()->DebugDrawTraceTag = TraceTag;
	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;
	*/

	//if (GetWorld()->SweepMultiByChannel(outHits, traceStart, traceEnd, rotation, collisionChannel, collisionShape, CollisionParams))
	if (GetWorld()->SweepMultiByChannel(outHits, traceStart, traceEnd, rotation, collisionChannel, collisionShape))
	{
		UE_LOG(LogBoardPiece, Warning, TEXT("ABoardPieceCPP::_DoPieceMove - Box Trace got something"));

		for (FHitResult hitResult : outHits)
		{
			if (!hitResult.Actor.IsValid())
			{
				continue;
			}

			UE_LOG(LogBoardPiece, Warning, TEXT("ABoardPieceCPP::_DoPieceMove - Hit: %s"), *(hitResult.GetActor()->GetFullName()));
			if (hitResult.GetActor()->GetClass()->IsChildOf(AF3MashUpCharacter::StaticClass()))
			{
				AF3MashUpCharacter* fpsChar = Cast<AF3MashUpCharacter>(hitResult.GetActor());
				fpsChar->CheckIfPlayerCrushed();
			}
		}
	}

	if (TimeSinceMovementStarted >= TimeToDoMove) {
		currentState = nextState;
		TimeSinceMovementStarted = 0.0f;

		if (currentState == BOARD_PIECE_STATE::IDLE) {
			_OnPieceMoveFinish();
		}
	}
}

void ABoardPieceCPP::_OnPieceMoveFinish()
{
	SwapInitiatingPlayerId = -1;
	OwningPieceHolder->ServerCheckForMatches();
}

void ABoardPieceCPP::AttemptSwap(ABoardPieceCPP* Other, int swappingPlayerId)
{
	OwningPieceHolder->DoSwap(Other->OwningPieceHolder, swappingPlayerId);
}

int ABoardPieceCPP::GetSwapInitiatingPlayerId()
{
	return SwapInitiatingPlayerId;
}
