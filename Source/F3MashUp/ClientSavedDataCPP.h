// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ClientSavedDataCPP.generated.h"

/**
 * 
 */
UCLASS()
class F3MASHUP_API UClientSavedDataCPP : public USaveGame
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;
};
