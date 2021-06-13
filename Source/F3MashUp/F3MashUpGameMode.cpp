// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "F3MashUpGameMode.h"
#include "F3MashUpHUD.h"
#include "F3MashUpCharacter.h"
#include "F3MashUpGameState.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogFpsMashUpGameMode, Warning, All);

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
}


void AF3MashUpGameMode::PlayerKilled(int KillerID, int VictimID)
{
	UE_LOG(LogFpsMashUpGameMode, Warning, TEXT("AF3MashUpGameMode::PlayerKilled - A player was killed. KillerID = %d, VictimID = %d"), KillerID, VictimID);

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

	// Copy the map to the game state player scores array
	AF3MashUpGameState* currentGameState = GetWorld()->GetGameState<AF3MashUpGameState>();
	currentGameState->PlayerScores.Empty();

	for(TPair<int,int> pair : PlayerScores) {
		FPlayerToScoreStruct ptsStruct = FPlayerToScoreStruct(pair.Key, pair.Value);
		currentGameState->PlayerScores.Add(ptsStruct);
	}
}
