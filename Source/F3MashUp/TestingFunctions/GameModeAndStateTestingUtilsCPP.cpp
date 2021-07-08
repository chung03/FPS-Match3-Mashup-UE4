// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModeAndStateTestingUtilsCPP.h"
#include "../F3MashUpGameMode.h"
#include "../F3MashUpGameState.h"
#include "../PlayerToScoreStruct.h"
#include "../BoardPieceCPP.h"
#include "../BoardPieceHolderCPP.h"
#include "../ScorePickUpCPP.h"
#include "../F3MashUpCharacter.h"
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

void UGameModeAndStateTestingUtilsCPP::ResetGameModeAndState(AFunctionalTest* FunctionalTestInstance)
{
	AGameStateBase* gameState = FunctionalTestInstance->GetWorld()->GetGameState();
	if (gameState)
	{
		AF3MashUpGameState* f3MashUpGameState = Cast<AF3MashUpGameState, AGameStateBase>(gameState);
		if (f3MashUpGameState)
		{
			f3MashUpGameState->ResetGameState();
		}
	}

	AGameModeBase* gameMode = FunctionalTestInstance->GetWorld()->GetAuthGameMode();
	if (gameMode)
	{
		AF3MashUpGameMode* f3MashUpGameMode = Cast<AF3MashUpGameMode, AGameModeBase>(gameMode);
		if (f3MashUpGameMode)
		{
			f3MashUpGameMode->ResetGameMode();
		}
	}
}

void UGameModeAndStateTestingUtilsCPP::CleanUpAllGameplayThings(AFunctionalTest* FunctionalTestInstance)
{
	ResetGameModeAndState(FunctionalTestInstance);

	RemoveAllActorsOfClass(FunctionalTestInstance, ABoardPieceCPP::StaticClass());
	RemoveAllActorsOfClass(FunctionalTestInstance, ABoardPieceHolderCPP::StaticClass());
	RemoveAllActorsOfClass(FunctionalTestInstance, AF3MashUpCharacter::StaticClass());
	RemoveAllActorsOfClass(FunctionalTestInstance, AScorePickUpCPP::StaticClass());

	UObject* ClassPackage = ANY_PACKAGE;
	UClass* Result = FindObject<UClass>(ClassPackage, TEXT("/Game/FirstPersonCPP/Blueprints/ScorePickups/ScorePickUpSpawnerBP.ScorePickUpSpawnerBP_C"));

	RemoveAllActorsOfClass(FunctionalTestInstance, Result);
}

void UGameModeAndStateTestingUtilsCPP::RemoveAllActorsOfClass(AFunctionalTest* FunctionalTestInstance, UClass* classType)
{
	UWorld* world = FunctionalTestInstance->GetWorld();

	TArray<AActor*> foundActors;
	UGameplayStatics::GetAllActorsOfClass(world, classType, foundActors);

	for (AActor * actor : foundActors)
	{
		if (actor && !actor->IsPendingKill()) {
			actor->Destroy();
		}
	}
}