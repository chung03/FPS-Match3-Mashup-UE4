// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerToScoreStruct.h"

FPlayerToScoreStruct::FPlayerToScoreStruct(int _PlayerId, int _PlayerScore, FString _PlayerName)
{
	PlayerId = _PlayerId;
	PlayerScore = _PlayerScore;
	PlayerName = _PlayerName;
}

FPlayerToScoreStruct::FPlayerToScoreStruct()
{
	PlayerId = -1;
	PlayerScore = 0;
	PlayerName = "DefaultPlayerName";
}

FPlayerToScoreStruct::~FPlayerToScoreStruct()
{
}
