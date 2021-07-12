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

	PlayerScores = TMap<int, int>();

	nextPlayerID = 0;
}


void AF3MashUpGameMode::PlayerKilled(int KillerID, int VictimID)
{
	UE_LOG(LogFpsMashUpGameMode, Log, TEXT("AF3MashUpGameMode::PlayerKilled - A player was killed. KillerID = %d, VictimID = %d"), KillerID, VictimID);

	if (KillerID != -1) {
		if (!PlayerScores.Contains(KillerID))
		{
			PlayerScores.Add(KillerID, 0);
		}

		int previousKillerScore = PlayerScores[KillerID];
		PlayerScores.Add(KillerID, previousKillerScore + 1);
	}

	if (!PlayerScores.Contains(VictimID))
	{
		PlayerScores.Add(VictimID, 0);
	}

	int previousVictimScore = PlayerScores[VictimID];
	PlayerScores.Add(VictimID, previousVictimScore - 1);

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
			PlayerScores.Add(playerID, 0);
		}


		int previousPlayerScore = PlayerScores[playerID];
		PlayerScores.Add(playerID, previousPlayerScore + scoreChange);

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

	for (TPair<int, int> pair : PlayerScores) {
		FPlayerToScoreStruct ptsStruct = FPlayerToScoreStruct(pair.Key, pair.Value);
		currentGameState->PlayerScores.Add(ptsStruct);
	}
}

bool AF3MashUpGameMode::ShouldMatchEnd()
{
	for (TPair<int, int> pair : PlayerScores) {
		if (pair.Value >= MatchPointsToWin)
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
	PlayerScores = TMap<int, int>();
}

void AF3MashUpGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AFPSMatch3PlayerControllerCPP* controller = Cast<AFPSMatch3PlayerControllerCPP, APlayerController>(NewPlayer);
	if (controller)
	{
		PlayerScores.Add(nextPlayerID, 0);
		controller->SetOwningPlayerID(nextPlayerID);
		++nextPlayerID;
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
		}
	}

	CopyScoresToGameState();
}