// Fill out your copyright notice in the Description page of Project Settings.


#include "F3MashUpGameInstance.h"

UF3MashUpGameInstance::UF3MashUpGameInstance()
{
	PlayerName = "";
	PlayerScoreStates = TArray<FPlayerToScoreStruct>();
}

void UF3MashUpGameInstance::ResetGameStats()
{
	PlayerScoreStates.Empty();
}