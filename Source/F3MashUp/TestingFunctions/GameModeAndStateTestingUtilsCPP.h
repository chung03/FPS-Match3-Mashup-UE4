// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FunctionalTest.h"
#include "../F3MashUpGameState.h"
#include "GameModeAndStateTestingUtilsCPP.generated.h"

/**
 * 
 */
UCLASS()
class F3MASHUP_API UGameModeAndStateTestingUtilsCPP : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	/*
	* All board pieces in the array will be connected
	* The assumption is that these board pieces are ordered
	* in the array such that they are in a line.
	* The first board piece will be connected to the second, the second
	* will be connected to the first and third, and so on.
	*/
	UFUNCTION(BlueprintCallable, Category = "Game Mode And Game State Testing")
	static void IsPlayerIdInGameState(int playerId, AFunctionalTest* FunctionalTestInstance);

	UFUNCTION(BlueprintCallable, Category = "Game Mode And Game State Testing")
	static void DoesPlayerHaveScore(int playerId, int expectedScore, AFunctionalTest* FunctionalTestInstance);

	UFUNCTION(BlueprintCallable, Category = "Game Mode And Game State Testing")
	static AF3MashUpGameState* GetGameStateForTest(AFunctionalTest* FunctionalTestInstance);

	UFUNCTION(BlueprintCallable, Category = "Game Mode And Game State Testing")
	static void ResetGameModeAndState(AFunctionalTest* FunctionalTestInstance);

	UFUNCTION(BlueprintCallable, Category = "Functional Testing Utils")
	static void CleanUpAllGameplayThings(AFunctionalTest* FunctionalTestInstance);

	static void RemoveAllActorsOfClass(AFunctionalTest* FunctionalTestInstance, UClass* classType);
};
