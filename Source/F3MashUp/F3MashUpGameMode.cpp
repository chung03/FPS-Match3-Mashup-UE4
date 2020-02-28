// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "F3MashUpGameMode.h"
#include "F3MashUpHUD.h"
#include "F3MashUpCharacter.h"
#include "UObject/ConstructorHelpers.h"

AF3MashUpGameMode::AF3MashUpGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AF3MashUpHUD::StaticClass();
}
