// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IPlayerInputAccepter.h"
#include "PlayerPawnInterface.h"
#include "NiagaraSystem.h"
#include "F3MashUpCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AF3MashUpCharacter : public ACharacter, public IPlayerInputAccepter, public IPlayerPawnInterface
{
	GENERATED_BODY()

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(Replicated)
	float Health;

	UPROPERTY(Replicated)
	bool CanFire;


	/** A timer handle used for providing the rate delay in-between shooting.*/
	FTimerHandle CanFireTimer;

	/** A timer handle used to let players shoot as long as they hold down the fire button*/
	FTimerHandle KeepFiringTimer;

	FVector InitialSpawnLocation;
	FRotator InitialSpawnRotation;

public:
	AF3MashUpCharacter();

protected:
	virtual void BeginPlay();

public:
	UPROPERTY(VisibleAnywhere, Replicated)
	float OwningPlayerID;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxHealth;

	/** How far away from the character the line trace will start.
	   This is to prevent line traces from hitting the body of the FPS character */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float LineTraceStartDistance = 20.0f;

	/** How far line traces will go */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float LineTraceEndDistance = 1000.0f;

	/** Turning Parameter for checking when the player is crushed*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	float CrushedCapsuleTraceFactor = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Gameplay)
	UNiagaraSystem* shotHitParticle;

	void CheckIfPlayerCrushed();

	void HandleFire_Implementation(bool isPressed) override;
	void HandleJump_Implementation(bool isPressed) override;
	void HandleMoveRight_Implementation(float axisValue) override;
	void HandleMoveForward_Implementation(float axisValue) override;
	void HandleTurn_Implementation(float axisValue) override;
	void HandleTurnRate_Implementation(float axisValue) override;
	void HandleLookUp_Implementation(float axisValue) override;
	void HandleLookUpRate_Implementation(float axisValue) override;
	void HandleResetVR_Implementation() override;
	void HandleRotateBoardPieceRight_Implementation(bool isPressed) override;
	void HandleRotateBoardPieceLeft_Implementation(bool isPressed) override;
	void HandleSetOwningPlayerID_Implementation(int newOwningPlayerID) override;

protected:
	UFUNCTION(BlueprintCallable)
	int GetOwningPlayerID();

	UFUNCTION(BlueprintCallable)
	void SetOwningPlayerID(int newOwningPlayerID);
	
	UFUNCTION(BlueprintCallable)
	float GetMaxHealth();

	UFUNCTION(BlueprintCallable)
	float GetHealth();

	UFUNCTION(BlueprintCallable)
	void StartFiring();

	UFUNCTION(BlueprintCallable)
	void StopFiring();

	/** Fires at a target. */
	UFUNCTION(BlueprintCallable)
	void OnFire();

	UFUNCTION(BlueprintCallable)
	void OnRotateBoardPiece(float degrees);

	/** Handles moving forward/backward */
	UFUNCTION(BlueprintCallable)
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	UFUNCTION(BlueprintCallable)
	void MoveRight(float Val);

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	UFUNCTION(BlueprintCallable)
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	UFUNCTION(BlueprintCallable)
	void LookUpAtRate(float Rate);
	
private:

	UFUNCTION(Server, Reliable)
	void ServerDoRotateBoardPiece(float degrees, FVector ForwardVector);

	void _DoRotateBoardPiece(float degrees, FVector ForwardVector);

	UFUNCTION(Server, Reliable)
	void ServerOnFire(FVector ForwardVector);

	void _OnFire(FVector ForwardVector);

	UFUNCTION(Server, Reliable)
	void ServerOnDamaged(float damage, int damagingPlayerId = -1);

	void _OnDamaged(float damage, int damagingPlayerId = -1);

	void AllowFire();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastCreateShotParticle(FVector particleLocation);


	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(Server, Reliable)
	void ServerDoCapsuleCheck();

	void _DoCapsuleCheck();

	void AllowServerDoCapsuleCheck();

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

public:
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};

