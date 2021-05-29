// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "F3MashUpGameMode.h"
#include "F3MashUpHUD.h"
#include "F3MashUpCharacter.h"
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
}


void AF3MashUpGameMode::PlayerKilled()
{
	UE_LOG(LogFpsMashUpGameMode, Warning, TEXT("AF3MashUpGameMode::PlayerKilled - A player was killed"));
}