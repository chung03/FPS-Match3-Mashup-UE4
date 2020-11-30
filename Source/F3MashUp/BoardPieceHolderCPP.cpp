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
	
	// Only Server should spawn a piece
	if (GetLocalRole() == ROLE_Authority) {
		SpawnNewBoardPiece();
	}
}

// Called every frame
void ABoardPieceHolderCPP::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABoardPieceHolderCPP::DoSwap(ABoardPieceHolderCPP* Other)
{
	
}

void ABoardPieceHolderCPP::CheckForMatches()
{
}

void ABoardPieceHolderCPP::SpawnNewBoardPiece_Implementation()
{
	// Only Server should spawn a piece
	//if (GetLocalRole() == ROLE_Authority) {
	//	return;
	//}

	int randomIndex = FMath::RandRange(0, PossibleBoardPieces.Num() - 1);
	TSubclassOf<class ABoardPieceCPP> newBoardPieceType = PossibleBoardPieces[randomIndex];

	FTransform transform = GetActorTransform();

	CurrentBoardPiece = GetWorld()->SpawnActor<ABoardPieceCPP>(newBoardPieceType.Get(), transform);
	CurrentBoardPiece->OwningPieceHolder = this;
}

void ABoardPieceHolderCPP::RemoveCurrentBoardPiece_Implementation()
{
	GetWorld()->DestroyActor(CurrentBoardPiece);
}