// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardPieceHolderCPP.h"
#include "BoardPieceCPP.h"

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
	// Swap ownership of pieces
	ABoardPieceCPP* temp = CurrentBoardPiece;
	CurrentBoardPiece = Other->CurrentBoardPiece;
	Other->CurrentBoardPiece = temp;

	// Swap location of the pieces
	FVector otherLocation = Other->CurrentBoardPiece->GetActorLocation();
	Other->CurrentBoardPiece->SetActorLocation(CurrentBoardPiece->GetActorLocation());
	CurrentBoardPiece->SetActorLocation(otherLocation);
}