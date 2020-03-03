// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardPieceHolderCPP.generated.h"

UCLASS()
class F3MASHUP_API ABoardPieceHolderCPP : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoardPieceHolderCPP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	const int NORTH_INDEX = 0;
	const int SOUTH_INDEX = 1;
	const int EAST_INDEX = 2;
	const int WEST_INDEX = 3;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*
	This is the array where connected Board Piece Holders should go.
	Board Piece Holders in this array are can be used for matches and swap board pieces with this Board Piece Holder.
	Each index has a specific meaning:
		0 = North
		1 = South
		2 = East
		3 = West
	An empty value in this array means that there is no connected Board Piece Holder in that direction. 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<ABoardPieceHolderCPP*> ConnectedBoardPieceHolders;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ABoardPieceCPP* CurrentBoardPiece;

	UFUNCTION(BlueprintCallable)
	void DoSwap(ABoardPieceHolderCPP* Other);
};