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
	float MatchPointsToWin = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int KillsToWin = 0;

	UFUNCTION(BlueprintCallable)
	void PlayerKilled(int KillerID, int VictimID);

	UFUNCTION(BlueprintCallable)
	void ChangeScoreOfPlayer(int playerID, int scoreChange);

	bool ShouldMatchEnd();

	void ResetGameMode();

protected:
	virtual void HandleMatchHasEnded() override;


private:
	UPROPERTY(EditAnywhere)
	TMap<int, int> PlayerScores;

	void CopyScoresToGameState();
};



