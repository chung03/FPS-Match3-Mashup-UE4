// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardPieceTestingUtilitiesCPP.h"



void UBoardPieceTestingUtilitiesCPP::SetConnectionsForBoardPieceHolders(TArray<ABoardPieceHolderCPP*> BoardPieces, bool IsNorthSouth, AFunctionalTest* FunctionalTestInstance)
{
	int prevPieceIndex = 0;
	int nextPieceIndex = 0;

	if (IsNorthSouth)
	{
		prevPieceIndex = 0;
		nextPieceIndex = 1;
	}
	else
	{
		prevPieceIndex = 2;
		nextPieceIndex = 3;
	}

	// Check for null board pieces here
	for (int index = 0; index < BoardPieces.Num(); ++index)
	{
		if (!BoardPieces[index])
		{
			FString failMessage ="UBoardPieceTestingUtilitiesCPP::SetConnectionsForBoardPieceHolders - A Board piece holder array had a NULL member. Index = " + FString::FromInt(index);
			FunctionalTestInstance->FinishTest(EFunctionalTestResult::Failed, failMessage);
			return;
		}
	}


	for (int index = 0; index < BoardPieces.Num(); ++index)
	{
		if (index - 1 >= 0)
		{
			BoardPieces[index]->ConnectedBoardPieceHolders[prevPieceIndex] = BoardPieces[index - 1];
		}
		
		if (index + 1 < BoardPieces.Num())
		{
			BoardPieces[index]->ConnectedBoardPieceHolders[nextPieceIndex] = BoardPieces[index + 1];
		}
	}
}


void UBoardPieceTestingUtilitiesCPP::IsBoardPieceOwnershipCorrect(ABoardPieceHolderCPP* BoardPieceHolder, ABoardPieceCPP* BoardPiece, AFunctionalTest* FunctionalTestInstance)
{
	FunctionalTestInstance->AssertEqual_Bool(BoardPieceHolder->CurrentBoardPiece == BoardPiece, true, "Board Piece Holder " + BoardPieceHolder->GetName() + " should think it owns BoardPiece " + BoardPiece->GetName(), nullptr);
	FunctionalTestInstance->AssertEqual_Bool(BoardPiece->OwningPieceHolder == BoardPieceHolder, true, "Board Piece " + BoardPiece->GetName() + " should think it is owned by BoardPieceHolder " + BoardPieceHolder->GetName(), nullptr);
}

void UBoardPieceTestingUtilitiesCPP::IsBoardPieceAtHolder(ABoardPieceHolderCPP* BoardPieceHolder, AFunctionalTest* FunctionalTestInstance)
{
	FunctionalTestInstance->AssertEqual_Vector(BoardPieceHolder->CurrentBoardPiece->GetActorLocation(), BoardPieceHolder->GetActorLocation(), "Board Piece should be in the same spot as the Board Piece Holder: " + BoardPieceHolder->GetName());
}