// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "PlayerToScoreStruct.h"
#include "FPSMatch3PlayerControllerCPP.h"
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

	UFUNCTION(BlueprintCallable)
	void ChangeNameOfPlayer(AFPSMatch3PlayerControllerCPP* controller, const FString& newPlayerName);

	bool ShouldMatchEnd();

	void ResetGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

protected:
	virtual void HandleMatchHasEnded() override;
	virtual void HandleMatchHasStarted() override;

private:
	UPROPERTY(EditAnywhere)
	TMap<int, FPlayerToScoreStruct> PlayerScores;

	UPROPERTY(EditAnywhere)
	TMap<AFPSMatch3PlayerControllerCPP*, int> PlayerControllerToIdMap;

	void CopyScoresToGameState();

	void ResetGameInstanceScoreState();

	int nextPlayerID;
};



