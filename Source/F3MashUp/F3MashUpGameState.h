// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "PlayerToScoreStruct.h"
#include "F3MashUpGameState.generated.h"

/**
 * 
 */
UCLASS()
class F3MASHUP_API AF3MashUpGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	TArray<FPlayerToScoreStruct> PlayerScores;
};
