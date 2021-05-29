// Fill out your copyright notice in the Description page of Project Settings.


#include "F3MashUpGameState.h"
#include "Net/UnrealNetwork.h"

void AF3MashUpGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AF3MashUpGameState, PlayerScores);
}