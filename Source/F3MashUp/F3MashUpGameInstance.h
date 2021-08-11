// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PlayerToScoreStruct.h"
#include "F3MashUpGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class F3MASHUP_API UF3MashUpGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UF3MashUpGameInstance();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FPlayerToScoreStruct> PlayerScoreStates;

	void ResetGameStats();
};
