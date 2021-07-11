// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "F3MashUpCharacter.h"
#include "F3MashUpProjectile.h"
#include "F3MashUpGameMode.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId
#include "BoardPieceCPP.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Log, All);

//////////////////////////////////////////////////////////////////////////
// AF3MashUpCharacter

AF3MashUpCharacter::AF3MashUpCharacter()
{
	//bReplicates = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AF3MashUpCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	Health = MaxHealth;
	CanFire = true;
	InitialSpawnLocation = GetActorLocation();
	InitialSpawnRotation = GetActorRotation();

	if (GetWorld()->IsServer())
	{
		GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AF3MashUpCharacter::OnHit);
	}
}

void AF3MashUpCharacter::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::OnHit - An object was hit"));

	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::OnHit - An object was hit")));
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, OtherActor->GetFullName());

	CheckIfPlayerCrushed();
}

void AF3MashUpCharacter::CheckIfPlayerCrushed()
{
	ServerDoCapsuleCheck();
}

void AF3MashUpCharacter::ServerDoCapsuleCheck_Implementation()
{
	_DoCapsuleCheck();
}

void AF3MashUpCharacter::_DoCapsuleCheck() {
	FVector scaledStartAndEnd = CrushedCapsuleTraceFactor * GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetCapsuleComponent()->GetUpVector();

	TArray<FHitResult> outHits;
	FVector traceStart = GetCapsuleComponent()->GetComponentLocation() + scaledStartAndEnd;

	FVector traceEnd = GetCapsuleComponent()->GetComponentLocation() - scaledStartAndEnd;

	FQuat rotation = FQuat().Identity;
	ECollisionChannel collisionChannel = ECollisionChannel::ECC_Pawn;
	FCollisionShape collisionShape = GetCapsuleComponent()->GetCollisionShape(5.0f);

	/*
	const FName TraceTag("MyTraceTag");

	GetWorld()->DebugDrawTraceTag = TraceTag;

	FCollisionQueryParams CollisionParams;
	CollisionParams.TraceTag = TraceTag;
	*/

	bool isTouchingFloorBelow = false;
	bool isTouchingPieceAbove = false;

	//if (GetWorld()->SweepMultiByChannel(outHits, traceStart, traceEnd, rotation, collisionChannel, collisionShape, CollisionParams))
	if (GetWorld()->SweepMultiByChannel(outHits, traceStart, traceEnd, rotation, collisionChannel, collisionShape))
	{
		UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Capsule Sweep got some hits: %d"), outHits.Num());

		int crushingBPSwapInitiatorID = -1;

		for (FHitResult hitResult : outHits)
		{
			UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Capsule Sweep Hit: %s"), *(hitResult.Actor->GetFullName()));

			if (hitResult.GetActor()->GetClass()->IsChildOf(ABoardPieceCPP::StaticClass()))
			{
				UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Board Piece found"));
				FVector bpLocation = hitResult.GetActor()->GetActorLocation();
				UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - BP Z = %f, Player Z = %f, Player Capsule Z = %f"), bpLocation.Z, GetActorLocation().Z, GetCapsuleComponent()->GetComponentLocation().Z);
				if (bpLocation.Z >= GetCapsuleComponent()->GetComponentLocation().Z)
				{
					UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Board Piece found was above the fps player"));
					isTouchingPieceAbove = true;

					ABoardPieceCPP * crushingBP = Cast<ABoardPieceCPP, AActor>(hitResult.GetActor());
					crushingBPSwapInitiatorID = crushingBP->GetSwapInitiatingPlayerId();
				}
			}
			else if (hitResult.GetActor()->ActorHasTag("Floor"))
			{
				UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Floor found"));
				FVector bpLocation = hitResult.GetActor()->GetActorLocation();
				if (bpLocation.Z <= GetCapsuleComponent()->GetComponentLocation().Z)
				{
					UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Floor found was below the fps player"));
					isTouchingFloorBelow = true;
				}
			}

			for (FName tag : hitResult.GetActor()->Tags) {
				UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - tag found = %s"), *(tag.ToString()));
			}

			UE_LOG(LogFPChar, Log, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - tag found? = %d"), hitResult.GetActor()->ActorHasTag("Floor"));
			
		}

		if (isTouchingPieceAbove && isTouchingFloorBelow)
		{
			ServerOnDamaged(100.0f, crushingBPSwapInitiatorID);
		}
	}
}


void AF3MashUpCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AF3MashUpCharacter, Health);
	DOREPLIFETIME(AF3MashUpCharacter, CanFire);
	DOREPLIFETIME(AF3MashUpCharacter, OwningPlayerID);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AF3MashUpCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Enable touchscreen input
	EnableTouchscreenMovement(PlayerInputComponent);
}

float AF3MashUpCharacter::GetMaxHealth()
{
	return MaxHealth;
}

float AF3MashUpCharacter::GetHealth()
{
	return Health;
}

int AF3MashUpCharacter::GetOwningPlayerID()
{
	return OwningPlayerID;
}

void AF3MashUpCharacter::SetOwningPlayerID(int newOwningPlayerID)
{
	OwningPlayerID = newOwningPlayerID;
}

void AF3MashUpCharacter::OnFire()
{
	// Get the rotation on client, but server will do the actual raycast
	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();

	ServerOnFire(ForwardVector);
}

void AF3MashUpCharacter::OnRotateBoardPiece(float degrees)
{
	// Get the rotation on client, but server will do the actual raycast
	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();

	ServerDoRotateBoardPiece(degrees, ForwardVector);
}

void AF3MashUpCharacter::ServerDoRotateBoardPiece_Implementation(float degrees, FVector ForwardVector)
{
	_DoRotateBoardPiece(degrees, ForwardVector);
}

void AF3MashUpCharacter::_DoRotateBoardPiece(float degrees, FVector ForwardVector)
{
	FVector Start = GetActorLocation() + (ForwardVector * LineTraceStartDistance);
	FVector End = ((ForwardVector * LineTraceEndDistance) + Start);

	FHitResult OutHit;

	// You can't shoot yourself, ignore yourself
	FCollisionQueryParams rotateTraceParams;
	rotateTraceParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Pawn, rotateTraceParams))
	{
		if (OutHit.GetActor()->GetClass()->IsChildOf(ABoardPieceCPP::StaticClass()))
		{
			FRotator rotator;
			rotator.Pitch = 0;
			rotator.Yaw = degrees;
			rotator.Roll = 0;
			OutHit.GetActor()->AddActorWorldRotation(rotator);
		}
	}
}

void AF3MashUpCharacter::ServerOnFire_Implementation(FVector ForwardVector)
{
	_OnFire(ForwardVector);
}

void AF3MashUpCharacter::_OnFire(FVector ForwardVector)
{
	if (!CanFire) {
		return;
	}

	CanFire = false;

	//FVector Start = FirstPersonCameraComponent->GetComponentLocation() + (ForwardVector * LineTraceStartDistance);
	FVector Start = GetActorLocation() + (ForwardVector * LineTraceStartDistance);
	FVector End = ((ForwardVector * LineTraceEndDistance) + Start);

	FHitResult OutHit;

	GetWorldTimerManager().SetTimer(CanFireTimer, this, &AF3MashUpCharacter::AllowFire, FireRate, false);

	// You can't shoot yourself, ignore yourself
	FCollisionQueryParams fireTraceParams;
	fireTraceParams.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Pawn, fireTraceParams))
	{
		AActor* hitActor = OutHit.GetActor();

		MulticastCreateShotParticle(OutHit.Location);

		if (hitActor && hitActor != this && hitActor->GetClass()->IsChildOf(AF3MashUpCharacter::StaticClass()))
		{
			AF3MashUpCharacter* other = Cast<AF3MashUpCharacter, AActor>(hitActor);

			other->ServerOnDamaged(1.0f, OwningPlayerID);
		}
		else if (hitActor && hitActor->GetClass()->IsChildOf(ABoardPieceCPP::StaticClass()))
		{
			ABoardPieceCPP* other = Cast<ABoardPieceCPP, AActor>(hitActor);

			other->ServerDamagePiece(OwningPlayerID, 1.0f);
		}
	}
}

void AF3MashUpCharacter::AllowFire()
{
	CanFire = true;
}

void AF3MashUpCharacter::StartFiring()
{
	OnFire();
	GetWorldTimerManager().SetTimer(KeepFiringTimer, this, &AF3MashUpCharacter::OnFire, FireRate, true);
}

void AF3MashUpCharacter::StopFiring()
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, "Timer should be cleared now");
	GetWorldTimerManager().ClearTimer(KeepFiringTimer);
}

void AF3MashUpCharacter::ServerOnDamaged_Implementation(float damage, int damagingPlayerId)
{
	_OnDamaged(damage, damagingPlayerId);
}

void AF3MashUpCharacter::_OnDamaged(float damage, int damagingPlayerId)
{
	Health -= damage;

	if (Health <= 0.0f)
	{
		AGameModeBase* gameMode = GetWorld() != NULL ? GetWorld()->GetAuthGameMode() : NULL;
		if (gameMode) 
		{
			AF3MashUpGameMode* f3MashUpGameMode = Cast<AF3MashUpGameMode, AGameModeBase>(gameMode);
			if (f3MashUpGameMode)
			{
				f3MashUpGameMode->PlayerKilled(damagingPlayerId, OwningPlayerID);
			}
		}
		Destroy();
	}
}

void AF3MashUpCharacter::MulticastCreateShotParticle_Implementation(FVector particleLocation)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		shotHitParticle,
		particleLocation,
		FRotator(1),
		FVector(1),
		true,
		true,
		ENCPoolMethod::AutoRelease,
		true);
}

void AF3MashUpCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AF3MashUpCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AF3MashUpCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void AF3MashUpCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AF3MashUpCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AF3MashUpCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AF3MashUpCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AF3MashUpCharacter::EnableTouchscreenMovement(class UInputComponent* PlayerInputComponent)
{
	if (FPlatformMisc::SupportsTouchInput() || GetDefault<UInputSettings>()->bUseMouseForTouch)
	{
		PlayerInputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AF3MashUpCharacter::BeginTouch);
		PlayerInputComponent->BindTouch(EInputEvent::IE_Released, this, &AF3MashUpCharacter::EndTouch);

		//Commenting this out to be more consistent with FPS BP template.
		//PlayerInputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AF3MashUpCharacter::TouchUpdate);
		return true;
	}
	
	return false;
}

void AF3MashUpCharacter::HandleFire_Implementation(bool isPressed)
{
	if (isPressed)
	{
		StartFiring();
	}
	else
	{
		StopFiring();
	}
}

void AF3MashUpCharacter::HandleJump_Implementation(bool isPressed)
{
	if (isPressed)
	{
		Jump();
	}
	else
	{
		StopJumping();
	}
}

void AF3MashUpCharacter::HandleMoveRight_Implementation(float axisValue)
{
	MoveRight(axisValue);
}

void AF3MashUpCharacter::HandleMoveForward_Implementation(float axisValue)
{
	MoveForward(axisValue);
}

void AF3MashUpCharacter::HandleTurn_Implementation(float axisValue)
{
	AddControllerYawInput(axisValue);
}

void AF3MashUpCharacter::HandleTurnRate_Implementation(float axisValue)
{
	TurnAtRate(axisValue);
}

void AF3MashUpCharacter::HandleLookUp_Implementation(float axisValue)
{
	AddControllerPitchInput(axisValue);
}

void AF3MashUpCharacter::HandleLookUpRate_Implementation(float axisValue)
{
	LookUpAtRate(axisValue);
}

void AF3MashUpCharacter::HandleResetVR_Implementation()
{
	OnResetVR();
}

void AF3MashUpCharacter::HandleRotateBoardPieceRight_Implementation(bool isPressed)
{
	if (isPressed)
	{
		OnRotateBoardPiece(90);
	}
}

void AF3MashUpCharacter::HandleRotateBoardPieceLeft_Implementation(bool isPressed)
{
	if (isPressed)
	{
		OnRotateBoardPiece(-90);
	}
}

void AF3MashUpCharacter::HandleSetOwningPlayerID_Implementation(int newOwningPlayerID)
{
	SetOwningPlayerID(newOwningPlayerID);
}