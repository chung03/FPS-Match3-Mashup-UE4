// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPlayerInputAccepter.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInputAccepter : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class F3MASHUP_API IPlayerInputAccepter
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleMatch3SelectObject(bool isPressed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleZoomInOut(float axisValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleFire(bool isPressed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleJump(bool isPressed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleMoveRight(float axisValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleMoveForward(float axisValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleTurn(float axisValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleTurnRate(float axisValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleLookUp(float axisValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleLookUpRate(float axisValue);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleResetVR();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleRotateBoardPieceRight(bool isPressed);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = PlayerInput)
	void HandleRotateBoardPieceLeft(bool isPressed);
};
