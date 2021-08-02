// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "F3MashUpGameMode.h"
#include "F3MashUpHUD.h"
#include "F3MashUpCharacter.h"
#include "F3MashUpGameState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "FPSMatch3PlayerControllerCPP.h"

DEFINE_LOG_CATEGORY_STATIC(LogFpsMashUpGameMode, Log, All);

AF3MashUpGameMode::AF3MashUpGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/Match3PawnBP"));
	
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AF3MashUpHUD::StaticClass();

	PlayerScores = TMap<int, FPlayerToScoreStruct>();
	PlayerControllerToIdMap = TMap<AFPSMatch3PlayerControllerCPP*, int>();

	nextPlayerID = 0;
}


void AF3MashUpGameMode::PlayerKilled(int KillerID, int VictimID)
{
	UE_LOG(LogFpsMashUpGameMode, Log, TEXT("AF3MashUpGameMode::PlayerKilled - A player was killed. KillerID = %d, VictimID = %d"), KillerID, VictimID);

	if (KillerID != -1) {
		if (!PlayerScores.Contains(KillerID))
		{
			UE_LOG(LogFpsMashUpGameMode, Error, TEXT("AF3MashUpGameMode::PlayerKilled - PlayerScores map did not contain the Player ID. All players should be initialized already. playerID = %d"), KillerID);
		}

		FPlayerToScoreStruct previousInfoStruct = PlayerScores[KillerID];
		previousInfoStruct.PlayerScore = previousInfoStruct.PlayerScore + 1;
		PlayerScores.Add(KillerID, previousInfoStruct);
	}

	if (!PlayerScores.Contains(VictimID))
	{
		UE_LOG(LogFpsMashUpGameMode, Error, TEXT("AF3MashUpGameMode::PlayerKilled - PlayerScores map did not contain the Player ID. All players should be initialized already. playerID = %d"), VictimID);
	}

	FPlayerToScoreStruct previousInfoStruct = PlayerScores[VictimID];
	previousInfoStruct.PlayerScore = previousInfoStruct.PlayerScore - 1;
	PlayerScores.Add(VictimID, previousInfoStruct);

	if (ShouldMatchEnd()) {
		EndMatch();
	}

	CopyScoresToGameState();
}

void AF3MashUpGameMode::ChangeScoreOfPlayer(int playerID, int scoreChange)
{
	UE_LOG(LogFpsMashUpGameMode, Log, TEXT("AF3MashUpGameMode::ChangeScoreOfPlayer - A player's score was changed. playerID = %d, scoreChange = %d"), playerID, scoreChange);

	if (playerID != -1) {
		if (!PlayerScores.Contains(playerID))
		{
			UE_LOG(LogFpsMashUpGameMode, Error, TEXT("AF3MashUpGameMode::ChangeScoreOfPlayer - PlayerScores map did not contain the Player ID. All players should be initialized already. playerID = %d"), playerID);
		}


		FPlayerToScoreStruct previousInfoStruct = PlayerScores[playerID];
		previousInfoStruct.PlayerScore = previousInfoStruct.PlayerScore + scoreChange;
		PlayerScores.Add(playerID, previousInfoStruct);

		if (ShouldMatchEnd()) {
			EndMatch();
		}

		CopyScoresToGameState();
	}
}

void AF3MashUpGameMode::CopyScoresToGameState()
{
	// Copy the map to the game state player scores array
	AF3MashUpGameState* currentGameState = GetWorld()->GetGameState<AF3MashUpGameState>();
	currentGameState->PlayerScores.Empty();

	for (TPair<int, FPlayerToScoreStruct> pair : PlayerScores) {
		FPlayerToScoreStruct ptsStruct = FPlayerToScoreStruct(pair.Key, pair.Value.PlayerScore, pair.Value.PlayerName);
		currentGameState->PlayerScores.Add(ptsStruct);
	}
}

bool AF3MashUpGameMode::ShouldMatchEnd()
{
	for (TPair<int, FPlayerToScoreStruct> pair : PlayerScores) {
		if (pair.Value.PlayerScore >= MatchPointsToWin)
		{
			return true;
		}
	}

	return false;
}


void AF3MashUpGameMode::HandleMatchHasEnded()
{
	Super::HandleMatchHasEnded();

	UE_LOG(LogFpsMashUpGameMode, Log, TEXT("AF3MashUpGameMode::HandleMatchHasEnded - The match has ended"));

	UGameplayStatics::OpenLevel(GetWorld(), "TitleScreen", true);
}


void AF3MashUpGameMode::ResetGameMode()
{
	PlayerScores = TMap<int, FPlayerToScoreStruct>();
	PlayerControllerToIdMap = TMap<AFPSMatch3PlayerControllerCPP*, int>();
}

void AF3MashUpGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AFPSMatch3PlayerControllerCPP* controller = Cast<AFPSMatch3PlayerControllerCPP, APlayerController>(NewPlayer);
	if (controller)
	{
		FPlayerToScoreStruct newStruct = FPlayerToScoreStruct(nextPlayerID, 0, "DefaultPlayerName");
		PlayerScores.Add(nextPlayerID, newStruct);
		controller->SetOwningPlayerID(nextPlayerID);
		PlayerControllerToIdMap.Add(controller, nextPlayerID);
		++nextPlayerID;

		controller->ClientSendPlayerName();
	}

	CopyScoresToGameState();
}

void AF3MashUpGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	AFPSMatch3PlayerControllerCPP* controller = Cast<AFPSMatch3PlayerControllerCPP, AController>(Exiting);
	if (controller)
	{
		if (!PlayerScores.Contains(controller->GetOwningPlayerID()))
		{
			PlayerScores.Remove(controller->GetOwningPlayerID());
			PlayerControllerToIdMap.Remove(controller);
		}
	}

	CopyScoresToGameState();
}

void AF3MashUpGameMode::ChangeNameOfPlayer(AFPSMatch3PlayerControllerCPP* controller, const FString& newPlayerName)
{
	UE_LOG(LogFpsMashUpGameMode, Log, TEXT("AF3MashUpGameMode::ChangeNameOfPlayer - A player's name was changed. newPlayerName = %s, Is controller null? = %d"), *newPlayerName, controller == nullptr);

	if (!PlayerControllerToIdMap.Contains(controller))
	{
		UE_LOG(LogFpsMashUpGameMode, Error, TEXT("AF3MashUpGameMode::ChangeNameOfPlayer - PlayerScores map did not contain the controller. All players should be initialized already. newPlayerName = %s, Is controller null? = %d"), *newPlayerName, controller == nullptr);
		return;
	}

	int playerId = PlayerControllerToIdMap[controller];

	FPlayerToScoreStruct previousInfoStruct = PlayerScores[playerId];
	previousInfoStruct.PlayerName = newPlayerName;
	PlayerScores.Add(playerId, previousInfoStruct);

	CopyScoresToGameState();
}