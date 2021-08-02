// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FPSMatch3PlayerControllerCPP.generated.h"

/**
 * 
 */
UCLASS()
class F3MASHUP_API AFPSMatch3PlayerControllerCPP : public APlayerController
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString PlayerName;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void SetOwningPlayerID(int newOwningPlayerID);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	int GetOwningPlayerID();

	UFUNCTION(Client, Reliable)
	void ClientSendPlayerName();

	UFUNCTION(Server, Reliable)
	void ServerSetPlayerNameInGameMode(const FString& newPlayerName);
};
