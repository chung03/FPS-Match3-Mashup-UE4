// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardPieceHolderCPP.h"
#include "BoardPieceCPP.h"
#include "Containers/Array.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"
#include "F3MashUpGameMode.h"

DEFINE_LOG_CATEGORY_STATIC(LogBoardPieceHolder, Log, All);

// Sets default values
ABoardPieceHolderCPP::ABoardPieceHolderCPP()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABoardPieceHolderCPP::BeginPlay()
{
	Super::BeginPlay();
	
	// Only Server should spawn a piece
	if (GetLocalRole() == ROLE_Authority) {
		ServerSpawnNewBoardPiece();

		// If Random spawn mode is enabled, start timers now.
		if (RandomSpawnMode == BOARDPIECEHOLDERMODE::RANDOM_SPAWN) {
			UWorld* World = GetWorld();
			World->GetTimerManager().SetTimer(PieceChangeTimer, this, &ABoardPieceHolderCPP::ServerReplaceCurrentBoardPiece, RandomChangeFrequency, true);
		}

		// If Random spawn mode is enabled, start timers now.
		if (RandomSpawnMode == BOARDPIECEHOLDERMODE::RANDOM_SWAP) {
			UWorld* World = GetWorld();
			World->GetTimerManager().SetTimer(PieceChangeTimer, this, &ABoardPieceHolderCPP::ServerRandomSwap, RandomChangeFrequency, true);
		}
	}
}

// Called every frame
void ABoardPieceHolderCPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABoardPieceHolderCPP::BeginDestroy()
{
	Super::BeginDestroy();

	if (CurrentBoardPiece) {
		GetWorld()->DestroyActor(CurrentBoardPiece);
	}
}

void ABoardPieceHolderCPP::ServerRandomSwap_Implementation()
{
	int randomIndex = FMath::RandRange(0, ConnectedBoardPieceHolders.Num() - 1);

	if (ConnectedBoardPieceHolders[randomIndex] != NULL) {
		_DoSwap(ConnectedBoardPieceHolders[randomIndex], -1);
	}
}

void ABoardPieceHolderCPP::DoSwap(ABoardPieceHolderCPP* Other, int swappingPlayerId)
{
	ServerDoSwap(Other, swappingPlayerId);
}

void ABoardPieceHolderCPP::ServerDoSwap_Implementation(ABoardPieceHolderCPP* Other, int swappingPlayerId)
{
	_DoSwap(Other, swappingPlayerId);
}

void ABoardPieceHolderCPP::_DoSwap(ABoardPieceHolderCPP* Other, int swappingPlayerId)
{
	if (!Other || !Other->IsSafeToChangePiece() || !IsSafeToChangePiece() || !IsConnectedPiece(Other)) {
		return;
	}

	// Start process of swapping pieces
	Other->CurrentBoardPiece->ServerDoSwapMovement(GetActorLocation(), true, swappingPlayerId);
	CurrentBoardPiece->ServerDoSwapMovement(Other->GetActorLocation(), false, swappingPlayerId);

	// Swap ownership of pieces
	ABoardPieceCPP* temp = CurrentBoardPiece;
	CurrentBoardPiece = Other->CurrentBoardPiece;
	Other->CurrentBoardPiece = temp;

	Other->CurrentBoardPiece->OwningPieceHolder = Other;
	CurrentBoardPiece->OwningPieceHolder = this;
}

void ABoardPieceHolderCPP::ServerCheckForMatches_Implementation()
{
	_CheckForMatches();
}

void ABoardPieceHolderCPP::_CheckForMatches()
{
	// No matches allowed when moving pieces
	if (!IsSafeToChangePiece()) {
		return;
	}

	// This board piece holder was not set up properly, don't do the match checking
	if (ConnectedBoardPieceHolders.Num() != 4) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("A board piece holder's connected board pieces array was not set up properly!")));
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, GetFullName());
		return;
	}

	// Find pieces with the same type in north and south directions
	TArray<ABoardPieceHolderCPP*> northSouthMatchingPieces = TArray<ABoardPieceHolderCPP*>();
	TArray<ABoardPieceHolderCPP*> eastWestMatchingPieces = TArray<ABoardPieceHolderCPP*>();

	_CheckNumMatchingPiecesInDirection(northSouthMatchingPieces, 0, 1);
	_CheckNumMatchingPiecesInDirection(eastWestMatchingPieces, 2, 3);

	bool isMatch = false;

	int totalNumMatchingPieces = northSouthMatchingPieces.Num() + eastWestMatchingPieces.Num();

	if (northSouthMatchingPieces.Num() >= 2) {
		for(ABoardPieceHolderCPP* other : northSouthMatchingPieces) {
			other->ServerReplaceCurrentBoardPiece();
		}

		isMatch = true;
	}

	if (eastWestMatchingPieces.Num() >= 2) {
		for (ABoardPieceHolderCPP* other : eastWestMatchingPieces) {
			other->ServerReplaceCurrentBoardPiece();
		}

		isMatch = true;
	}

	if (isMatch) {
		int swapInitiatingID = CurrentBoardPiece->GetSwapInitiatingPlayerId();
		AGameModeBase* gameMode = GetWorld() != NULL ? GetWorld()->GetAuthGameMode() : NULL;
		if (gameMode)
		{
			AF3MashUpGameMode* f3MashUpGameMode = Cast<AF3MashUpGameMode, AGameModeBase>(gameMode);
			if (f3MashUpGameMode)
			{
				f3MashUpGameMode->ChangeScoreOfPlayer(swapInitiatingID, (totalNumMatchingPieces + 1) * PointsPerBoardPieceMatched);
			}
		}

		ServerReplaceCurrentBoardPiece();
	}
}

void ABoardPieceHolderCPP::_CheckNumMatchingPiecesInDirection(TArray<ABoardPieceHolderCPP*> &MatchingPieces, int directionIndex1, int directionIndex2)
{
	ABoardPieceHolderCPP* nextDirectionPieceHolder = ConnectedBoardPieceHolders[directionIndex1];
	while (nextDirectionPieceHolder && IsSamePieceType(nextDirectionPieceHolder))
	{
		MatchingPieces.Add(nextDirectionPieceHolder);
		nextDirectionPieceHolder = nextDirectionPieceHolder->ConnectedBoardPieceHolders[directionIndex1];
	}

	nextDirectionPieceHolder = ConnectedBoardPieceHolders[directionIndex2];
	while (nextDirectionPieceHolder && IsSamePieceType(nextDirectionPieceHolder))
	{
		MatchingPieces.Add(nextDirectionPieceHolder);
		nextDirectionPieceHolder = nextDirectionPieceHolder->ConnectedBoardPieceHolders[directionIndex2];
	}
}

void ABoardPieceHolderCPP::ServerSpawnNewBoardPiece_Implementation()
{
	_SpawnNewBoardPiece();
}

void ABoardPieceHolderCPP::_SpawnNewBoardPiece()
{
	int randomIndex = FMath::RandRange(0, PossibleBoardPieces.Num() - 1);
	TSubclassOf<class ABoardPieceCPP> newBoardPieceType = PossibleBoardPieces[randomIndex];

	FTransform transform = GetActorTransform();

	FActorSpawnParameters spawnParams;
	//spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	//UE_LOG(LogBoardPieceHolder, Log, TEXT("ABoardPieceHolderCPP::_SpawnNewBoardPiece - New Piece Spawn Location = %s"), *(transform.GetLocation().ToString()));

	CurrentBoardPiece = GetWorld()->SpawnActor<ABoardPieceCPP>(newBoardPieceType.Get(), transform);
	CurrentBoardPiece->OwningPieceHolder = this;

	int randomRotationFactor = FMath::RandRange(0, 3);
	FRotator rotateAngles = FRotator(0, 90 * randomRotationFactor, 0);

	CurrentBoardPiece->SetActorRotation(rotateAngles);
	CurrentBoardPiece->ServerDoSpawnMovement();
}

void ABoardPieceHolderCPP::ServerReplaceCurrentBoardPiece_Implementation()
{
	_DestroyBoardPiece();
	_SpawnNewBoardPiece();
}

void ABoardPieceHolderCPP::_DestroyBoardPiece()
{
	if (CurrentBoardPiece && IsSafeToChangePiece()) {
		GetWorld()->DestroyActor(CurrentBoardPiece);
	}
}

bool ABoardPieceHolderCPP::IsSafeToChangePiece()
{
	if (CurrentBoardPiece && !CurrentBoardPiece->IsPieceMoving()) {
		return true;
	}

	return false;
}

bool ABoardPieceHolderCPP::IsConnectedPiece(ABoardPieceHolderCPP* Other)
{
	for (int index = 0; index < ConnectedBoardPieceHolders.Num(); index++)
	{
		if (Other == ConnectedBoardPieceHolders[index])
		{
			return true;
		}
	}

	return false;
}

bool ABoardPieceHolderCPP::IsSamePieceType(ABoardPieceHolderCPP* Other)
{
	if (Other && Other->IsSafeToChangePiece() && CurrentBoardPiece && CurrentBoardPiece->IsSameType(Other->CurrentBoardPiece))
	{
		return true;
	}

	return false;
}