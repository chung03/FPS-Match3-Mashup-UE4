// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndStateTestingUtilsCPP.h"
#include "../F3MashUpGameMode.h"
#include "../F3MashUpGameState.h"
#include "../PlayerToScoreStruct.h"
#include "Kismet/GameplayStatics.h"

void UGameModeAndStateTestingUtilsCPP::IsPlayerIdInGameState(int playerId, AFunctionalTest* FunctionalTestInstance)
{
	bool playerIdIsFound = false;

	AF3MashUpGameState* f3MashUpGameState = GetGameStateForTest(FunctionalTestInstance);
	if (f3MashUpGameState)
	{
		for (FPlayerToScoreStruct playerScore : f3MashUpGameState->PlayerScores)
		{
			if (playerScore.PlayerId == playerId)
			{
				playerIdIsFound = true;
			}
		}
	}


	if (!playerIdIsFound)
	{
		FString failMessage = "UGameModeAndStateTestingUtilsCPP::IsPlayerIdInGameState - Player ID was not found in Game State Score Array. Player ID = " + FString::FromInt(playerId);
		FunctionalTestInstance->FinishTest(EFunctionalTestResult::Failed, failMessage);
	}
}

void UGameModeAndStateTestingUtilsCPP::DoesPlayerHaveScore(int playerId, int expectedScore, AFunctionalTest* FunctionalTestInstance)
{
	IsPlayerIdInGameState(playerId, FunctionalTestInstance);

	AF3MashUpGameState* f3MashUpGameState = GetGameStateForTest(FunctionalTestInstance);
	if (f3MashUpGameState)
	{
		for (FPlayerToScoreStruct playerScore : f3MashUpGameState->PlayerScores)
		{
			if (playerScore.PlayerId == playerId)
			{
				FunctionalTestInstance->AssertEqual_Int(playerScore.PlayerScore, expectedScore, "UGameModeAndStateTestingUtilsCPP::DoesPlayerHaveScore - The player's score did not change as expected");
			}
		}
	}
}


AF3MashUpGameState* UGameModeAndStateTestingUtilsCPP::GetGameStateForTest(AFunctionalTest* FunctionalTestInstance)
{
	AGameStateBase* gameState = FunctionalTestInstance->GetWorld()->GetGameState();
	if (gameState)
	{
		AF3MashUpGameState* f3MashUpGameState = Cast<AF3MashUpGameState, AGameStateBase>(gameState);
		if (f3MashUpGameState)
		{
			return f3MashUpGameState;
		}
		else
		{
			FString failMessage = "UGameModeAndStateTestingUtilsCPP::GetGameStateForTest - Unable to cast game state to AF3MashUpGameState";
			FunctionalTestInstance->FinishTest(EFunctionalTestResult::Failed, failMessage);
		}
	}
	else
	{
		FString failMessage = "UGameModeAndStateTestingUtilsCPP::GetGameStateForTest - Unable to get game state";
		FunctionalTestInstance->FinishTest(EFunctionalTestResult::Failed, failMessage);
	}

	return nullptr;
}