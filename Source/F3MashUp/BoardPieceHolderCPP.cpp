// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardPieceHolderCPP.h"
#include "BoardPieceCPP.h"
#include "Containers/Array.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/World.h"

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
	
}

// Called every frame
void ABoardPieceHolderCPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABoardPieceHolderCPP::DoSwap(ABoardPieceHolderCPP* Other)
{
	if (Other == NULL) {
		return;
	}

	// Swap ownership of pieces
	ABoardPieceCPP* temp = CurrentBoardPiece;
	CurrentBoardPiece = Other->CurrentBoardPiece;
	Other->CurrentBoardPiece = temp;

	// Swap location of the pieces
	FVector otherLocation = Other->CurrentBoardPiece->GetActorLocation();
	Other->CurrentBoardPiece->SetActorLocation(CurrentBoardPiece->GetActorLocation());
	CurrentBoardPiece->SetActorLocation(otherLocation);
}

void ABoardPieceHolderCPP::CheckForMatches()
{
	/*
	Check for matches by checking for matching pieces in all 4 directions
	If there are enough pieces that match (North-to-South and East-to-West), then remove them
	*/

	// Get all the matching pieces now in all directions
	TArray<ABoardPieceHolderCPP*> NorthSouthMatchingBoardPieces = TArray<ABoardPieceHolderCPP*>();
	ABoardPieceHolderCPP* currentNorthPiece = ConnectedBoardPieceHolders[NORTH_INDEX];
	while (currentNorthPiece != NULL && currentNorthPiece->CurrentBoardPiece->IsSameType(CurrentBoardPiece))
	{
		NorthSouthMatchingBoardPieces.Add(currentNorthPiece);
		currentNorthPiece = currentNorthPiece->ConnectedBoardPieceHolders[NORTH_INDEX];
	}

	ABoardPieceHolderCPP* currentSouthPiece = ConnectedBoardPieceHolders[SOUTH_INDEX];
	while (currentSouthPiece != NULL && currentSouthPiece->CurrentBoardPiece->IsSameType(CurrentBoardPiece))
	{
		NorthSouthMatchingBoardPieces.Add(currentSouthPiece);
		currentSouthPiece = currentSouthPiece->ConnectedBoardPieceHolders[SOUTH_INDEX];
	}

	TArray<ABoardPieceHolderCPP*> EastWestMatchingBoardPieces = TArray<ABoardPieceHolderCPP*>();
	ABoardPieceHolderCPP* currentEastPiece = ConnectedBoardPieceHolders[EAST_INDEX];
	while (currentEastPiece != NULL && currentEastPiece->CurrentBoardPiece->IsSameType(CurrentBoardPiece))
	{
		EastWestMatchingBoardPieces.Add(currentEastPiece);
		currentEastPiece = currentEastPiece->ConnectedBoardPieceHolders[EAST_INDEX];
	}

	ABoardPieceHolderCPP* currentWestPiece = ConnectedBoardPieceHolders[WEST_INDEX];
	while (currentWestPiece != NULL && currentWestPiece->CurrentBoardPiece->IsSameType(CurrentBoardPiece))
	{
		EastWestMatchingBoardPieces.Add(currentWestPiece);
		currentWestPiece = currentWestPiece->ConnectedBoardPieceHolders[WEST_INDEX];
	}

	bool isMatch = false;

	// Replace those pieces if there is a match
	if (NorthSouthMatchingBoardPieces.Num() >= 2) {
		isMatch = true;
		for (TCheckedPointerIterator< ABoardPieceHolderCPP*, int32 > it = NorthSouthMatchingBoardPieces.begin(); it != NorthSouthMatchingBoardPieces.end(); ++it) {
			(*it)->ReplaceCurrentBoardPiece();
		}
	}

	if (EastWestMatchingBoardPieces.Num() >= 2) {
		isMatch = true;
		for (TCheckedPointerIterator< ABoardPieceHolderCPP*, int32 > it = EastWestMatchingBoardPieces.begin(); it != EastWestMatchingBoardPieces.end(); ++it) {
			(*it)->ReplaceCurrentBoardPiece();
		}
	}

	if (isMatch) {
		ReplaceCurrentBoardPiece();
	}
}

void ABoardPieceHolderCPP::SpawnNewBoardPiece()
{
	int randomIndex = FMath::RandRange(0, PossibleBoardPieces.Num() - 1);
	TSubclassOf<class ABoardPieceCPP> newBoardPieceType = PossibleBoardPieces[randomIndex];

	FTransform transform = GetActorTransform();

	CurrentBoardPiece = GetWorld()->SpawnActor<ABoardPieceCPP>(newBoardPieceType.Get(), transform);
	CurrentBoardPiece->OwningPieceHolder = this;
}

void ABoardPieceHolderCPP::ReplaceCurrentBoardPiece()
{
	GetWorld()->DestroyActor(CurrentBoardPiece);
	SpawnNewBoardPiece();
}