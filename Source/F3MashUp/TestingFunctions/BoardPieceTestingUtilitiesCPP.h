// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "../BoardPieceHolderCPP.h"
#include "../BoardPieceCPP.h"
#include "FunctionalTest.h"
#include "BoardPieceTestingUtilitiesCPP.generated.h"

/**
 * 
 */
UCLASS()
class F3MASHUP_API UBoardPieceTestingUtilitiesCPP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/*
	* All board pieces in the array will be connected
	* The assumption is that these board pieces are ordered 
	* in the array such that they are in a line.
	* The first board piece will be connected to the second, the second
	* will be connected to the first and third, and so on.
	*/
	UFUNCTION(BlueprintCallable, Category = "Board Piece Testing")
	static void SetConnectionsForBoardPieceHolders(TArray<ABoardPieceHolderCPP*> BoardPieces, bool IsNorthSouth, AFunctionalTest* FunctionalTestInstance);

	/*
	* Checks that BoardPieceHolder and BoardPiece both think
	* that BoardPieceHolder owns BoardPiece
	*/
	UFUNCTION(BlueprintCallable, Category = "Board Piece Testing")
	static void IsBoardPieceOwnershipCorrect(ABoardPieceHolderCPP* BoardPieceHolder, ABoardPieceCPP* BoardPiece, AFunctionalTest* FunctionalTestInstance);

	/*
	* Checks that BoardPieceHolder its BoardPiece are in the same place
	*/
	UFUNCTION(BlueprintCallable, Category = "Board Piece Testing")
	static void IsBoardPieceAtHolder(ABoardPieceHolderCPP* BoardPieceHolder, AFunctionalTest* FunctionalTestInstance);
};
