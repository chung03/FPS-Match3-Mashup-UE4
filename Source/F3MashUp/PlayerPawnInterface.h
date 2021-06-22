// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerPawnInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerPawnInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class F3MASHUP_API IPlayerPawnInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerPawnCommon)
	void HandleSetOwningPlayerID(int newOwningPlayerID);
};
