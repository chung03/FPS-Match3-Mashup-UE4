// Fill out your copyright notice in the Description page of Project Settings.


#include "F3MashUpGameState.h"
#include "Net/UnrealNetwork.h"
#include "PlayerToScoreStruct.h"

DEFINE_LOG_CATEGORY_STATIC(LogFpsMashUpGameState, Warning, All);

void AF3MashUpGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	UE_LOG(LogFpsMashUpGameState, Warning, TEXT("AF3MashUpGameState::GetLifetimeReplicatedProps - Properties were replicated"));

	DOREPLIFETIME(AF3MashUpGameState, PlayerScores);
}

void AF3MashUpGameState::ResetGameState()
{
	PlayerScores = TArray<FPlayerToScoreStruct>();
}