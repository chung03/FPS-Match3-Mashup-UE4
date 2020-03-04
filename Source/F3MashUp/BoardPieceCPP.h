// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardPieceCPP.generated.h"

UCLASS()
class F3MASHUP_API ABoardPieceCPP : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABoardPieceCPP();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class ABoardPieceHolderCPP* OwningPieceHolder;

	/*
	Quick way to get Board Piece Types working. Each Board Piece Type should have a different value for this
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int BoardPieceType = 0;

	bool IsSameType(ABoardPieceCPP* Other);
};