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

	UFUNCTION(BlueprintCallable)
	void PlayerKilled();
};



