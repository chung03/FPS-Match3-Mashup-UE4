// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSMatch3PlayerControllerCPP.h"
#include "F3MashUpGameMode.h"
#include "ClientSavedDataCPP.h"
#include "F3MashUpGameInstance.h"
#include "Kismet/GameplayStatics.h"

void AFPSMatch3PlayerControllerCPP::ClientSendPlayerName_Implementation()
{
	UGameInstance* gameInstance = GetGameInstance();
	UF3MashUpGameInstance* f3MashUpGameInstance = Cast<UF3MashUpGameInstance>(gameInstance);
	if (f3MashUpGameInstance)
	{
		// The operation was successful, so LoadedGame now contains the data we saved earlier.
		UE_LOG(LogTemp, Warning, TEXT("LOADED: %s"), *f3MashUpGameInstance->PlayerName);

		PlayerName = f3MashUpGameInstance->PlayerName;
	}

	ServerSetPlayerNameInGameMode(PlayerName);
}

void AFPSMatch3PlayerControllerCPP::ServerSetPlayerNameInGameMode_Implementation(const FString& newPlayerName)
{
	AGameModeBase* gameMode = GetWorld() != NULL ? GetWorld()->GetAuthGameMode() : NULL;
	if (gameMode)
	{
		AF3MashUpGameMode* f3MashUpGameMode = Cast<AF3MashUpGameMode, AGameModeBase>(gameMode);
		if (f3MashUpGameMode)
		{
			f3MashUpGameMode->ChangeNameOfPlayer(this, newPlayerName);
		}
	}
}