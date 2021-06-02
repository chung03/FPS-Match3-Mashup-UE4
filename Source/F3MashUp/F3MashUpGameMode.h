// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "F3MashUpGameMode.generated.h"

UCLASS(minimalapi)
class AF3MashUpGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AF3MashUpGameMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MatchLengthMinutes = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int KillsToWin = 0;

	UFUNCTION(BlueprintCallable)
	void PlayerKilled(int KillerID, int VictimID);

private:
	UPROPERTY(EditAnywhere)
	TMap<int, int> PlayerScores;
};



