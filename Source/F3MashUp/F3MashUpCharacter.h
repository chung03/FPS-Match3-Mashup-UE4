// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IPlayerInputAccepter.h"
#include "PlayerPawnInterface.h"
#include "F3MashUpCharacter.generated.h"

class UInputComponent;

UCLASS(config=Game)
class AF3MashUpCharacter : public ACharacter, public IPlayerInputAccepter, public IPlayerPawnInterface
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	class USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* FP_MuzzleLocation;

	/** Gun mesh: VR view (attached to the VR controller directly, no arm, just the actual gun) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USkeletalMeshComponent* VR_Gun;

	/** Location on VR gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	class USceneComponent* VR_MuzzleLocation;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	/** Motion controller (right hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* R_MotionController;

	/** Motion controller (left hand) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent* L_MotionController;

	UPROPERTY(Replicated)
	float Health;

	UPROPERTY(Replicated)
	bool CanFire;
	
	UPROPERTY(Replicated)
	bool CanServerDoCapsuleCheck;


	/** A timer handle used for providing the rate delay in-between shooting.*/
	FTimerHandle CanFireTimer;

	/** A timer handle used to let players shoot as long as they hold down the fire button*/
	FTimerHandle KeepFiringTimer;

	/** A timer handle used to control how often the server is asked to check if the player got crushed*/
	FTimerHandle ServerCapsuleCheckTimer;

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

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MaxHealth;

	/** Whether to use motion controller location for aiming. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	uint32 bUsingMotionControllers : 1;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float ServerCapsuleCheckRate;

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

	/** Resets HMD orientation and position in VR. */
	UFUNCTION(BlueprintCallable)
	void OnResetVR();

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

	struct TouchData
	{
		TouchData() { bIsPressed = false;Location=FVector::ZeroVector;}
		bool bIsPressed;
		ETouchIndex::Type FingerIndex;
		FVector Location;
		bool bMoved;
	};
	void BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location);
	void TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location);
	TouchData	TouchItem;
	
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

	/* 
	 * Configures input for touchscreen devices if there is a valid touch interface for doing so 
	 *
	 * @param	InputComponent	The input component pointer to bind controls to
	 * @returns true if touch controls were enabled.
	 */
	bool EnableTouchscreenMovement(UInputComponent* InputComponent);

public:
	/** Returns Mesh1P subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
};

