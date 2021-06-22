// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerToScoreStruct.h"

FPlayerToScoreStruct::FPlayerToScoreStruct(int _PlayerId, int _PlayerScore)
{
	PlayerId = _PlayerId;
	PlayerScore = _PlayerScore;
}

FPlayerToScoreStruct::FPlayerToScoreStruct()
{
	PlayerId = -1;
	PlayerScore = 0;
}

FPlayerToScoreStruct::~FPlayerToScoreStruct()
{
}
