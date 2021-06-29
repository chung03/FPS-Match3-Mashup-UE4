// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardPieceCPP.generated.h"

UCLASS()
class F3MASHUP_API ABoardPieceCPP : public AActor
{
	GENERATED_BODY()

	enum class BOARD_PIECE_STATE {
		IDLE,
		SPAWNING,
		SWAP_UNDER_WAIT,
		SWAP_UNDER,
		SWAP_OVER_1,
		SWAP_OVER_2,
		SWAP_OVER_3
	};
	
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoardPieceSpawnHeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoardPieceSwapHeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeToFinishSpawnMovement = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeToFinishSwapUnderMovement = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeToFinishSwapUnderWait = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeToFinishSwapOverStepMovement = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoardPieceMaxHp = 0;

	bool IsSameType(ABoardPieceCPP* Other);

	UFUNCTION(Server, Reliable)
	void ServerDoSpawnMovement();

	void _DoSpawnMovement();

	UFUNCTION(Server, Reliable)
	void ServerDoSwapMovement(FVector TargetLocation, bool isMovingUnder, int swappingPlayerId);

	void _DoSwapMovement(FVector TargetLocation, bool isMovingUnder, int swappingPlayerId);

	bool IsPieceMoving();

	UFUNCTION(BlueprintCallable)
	void AttemptSwap(ABoardPieceCPP* Other, int swappingPlayerId);

	int GetSwapInitiatingPlayerId();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerDamagePiece(int damagingPlayerId, float damage);

	void _DamagePiece(int damagingPlayerId, float damage);

private:
	FVector RootLocation;

	FVector SwapTargetLocation;
	
	float TimeSinceMovementStarted;

	void _DoPieceMove(FVector TargetLocation, FVector StartingLocation, BOARD_PIECE_STATE nextState, float TimeToDoMove);

	void _OnPieceMoveFinish();

	UFUNCTION()
	void _HandleOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	BOARD_PIECE_STATE currentState = BOARD_PIECE_STATE::IDLE;

	UStaticMeshComponent* _staticMesh;

	int SwapInitiatingPlayerId;

	UPROPERTY(Replicated)
	float BoardPieceCurrentHp;

	UPROPERTY(Replicated)
	bool isDying;
};