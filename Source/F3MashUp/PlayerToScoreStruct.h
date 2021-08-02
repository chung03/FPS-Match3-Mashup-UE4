// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerToScoreStruct.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct F3MASHUP_API FPlayerToScoreStruct
{
	GENERATED_USTRUCT_BODY()

public:
	FPlayerToScoreStruct(int _PlayerId, int _PlayerScore, FString _PlayerName);
	FPlayerToScoreStruct();
	~FPlayerToScoreStruct();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PlayerId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int PlayerScore;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;
};
