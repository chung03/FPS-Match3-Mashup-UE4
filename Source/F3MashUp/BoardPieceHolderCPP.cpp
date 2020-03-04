// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardPieceHolderCPP.h"
#include "BoardPieceCPP.h"
#include "Containers/Array.h"

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
	bool isMatch = false;

	// Check for matches where this board piece is in the middle
	if (ConnectedBoardPieceHolders[NORTH_INDEX] != NULL && ConnectedBoardPieceHolders[SOUTH_INDEX] != NULL)
	{
		if (ConnectedBoardPieceHolders[NORTH_INDEX]->CurrentBoardPiece->IsSameType(CurrentBoardPiece)
			&& ConnectedBoardPieceHolders[SOUTH_INDEX]->CurrentBoardPiece->IsSameType(CurrentBoardPiece)) {
			isMatch = true;
		}
	}

	if (ConnectedBoardPieceHolders[EAST_INDEX] != NULL && ConnectedBoardPieceHolders[WEST_INDEX] != NULL)
	{
		if (ConnectedBoardPieceHolders[EAST_INDEX]->CurrentBoardPiece->IsSameType(CurrentBoardPiece)
			&& ConnectedBoardPieceHolders[WEST_INDEX]->CurrentBoardPiece->IsSameType(CurrentBoardPiece)) {
			isMatch = true;
		}
	}

	/*
	Check for matches by checking for matching pieces in all 4 directions
	If there are enough pieces that match (North-to-South and East-to-West), then remove them
	*/

	// Get all the matching pieces now in all directions
	TArray<ABoardPieceHolderCPP*> NorthSouthMatchingBoardPieces = TArray<ABoardPieceHolderCPP*>();
	ABoardPieceHolderCPP* currentNorthPiece = ConnectedBoardPieceHolders[NORTH_INDEX];
	while (currentNorthPiece != NULL)
	{
		NorthSouthMatchingBoardPieces.Add(currentNorthPiece);
		currentNorthPiece = currentNorthPiece->ConnectedBoardPieceHolders[NORTH_INDEX];
	}

	ABoardPieceHolderCPP* currentSouthPiece = ConnectedBoardPieceHolders[SOUTH_INDEX];
	while (currentSouthPiece != NULL)
	{
		NorthSouthMatchingBoardPieces.Add(currentSouthPiece);
		currentSouthPiece = currentNorthPiece->ConnectedBoardPieceHolders[SOUTH_INDEX];
	}

	TArray<ABoardPieceHolderCPP*> EastWestMatchingBoardPieces = TArray<ABoardPieceHolderCPP*>();
	ABoardPieceHolderCPP* currentEastPiece = ConnectedBoardPieceHolders[EAST_INDEX];
	while (currentEastPiece != NULL)
	{
		EastWestMatchingBoardPieces.Add(currentEastPiece);
		currentEastPiece = currentEastPiece->ConnectedBoardPieceHolders[EAST_INDEX];
	}

	ABoardPieceHolderCPP* currentWestPiece = ConnectedBoardPieceHolders[WEST_INDEX];
	while (currentWestPiece != NULL)
	{
		EastWestMatchingBoardPieces.Add(currentWestPiece);
		currentWestPiece = currentWestPiece->ConnectedBoardPieceHolders[WEST_INDEX];
	}

	// Replace those pieces if there is a match
	if (NorthSouthMatchingBoardPieces.Num() >= 2) {
		for (TCheckedPointerIterator< ABoardPieceHolderCPP*, int32 > it = NorthSouthMatchingBoardPieces.begin(); it != NorthSouthMatchingBoardPieces.end(); ++it) {
			//(*it)->
		}
	}

	if (EastWestMatchingBoardPieces.Num() >= 2) {

	}
}