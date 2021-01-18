// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "F3MashUpCharacter.h"
#include "F3MashUpProjectile.h"
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

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

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

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void AF3MashUpCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}

	Health = MaxHealth;
	CanFire = true;
	InitialSpawnLocation = GetActorLocation();
	InitialSpawnRotation = GetActorRotation();

	GetCapsuleComponent()->OnComponentHit.AddDynamic(this, &AF3MashUpCharacter::OnHit);
	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &AF3MashUpCharacter::OnBeginOverlap);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &AF3MashUpCharacter::OnEndOverlap);

}

void AF3MashUpCharacter::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::OnHit - An object was hit"));

	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::OnHit - An object was hit")));
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, OtherActor->GetFullName());

	_DoCapsuleCheck();
}

void AF3MashUpCharacter::OnBeginOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::OnBeginOverlap - An object started overlap"));

	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::OnBeginOverlap - An object started overlap")));
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, OtherActor->GetFullName());

	_DoCapsuleCheck();
}

void AF3MashUpCharacter::OnEndOverlap(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::OnEndOverlap - An object ended overlap"));

	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::OnEndOverlap - An object ended overlap")));
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, OtherActor->GetFullName());

	_DoCapsuleCheck();
}

void AF3MashUpCharacter::CheckIfPlayerCrushed()
{
	_DoCapsuleCheck();
}

void AF3MashUpCharacter::_DoCapsuleCheck() {
	FVector scaledStartAndEnd = CrushedCapsuleTraceFactor * GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetCapsuleComponent()->GetUpVector();
	//FVector scaledStartAndEnd = CrushedCapsuleTraceFactor * GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * GetCapsuleComponent()->GetUpVector();

	TArray<FHitResult> outHits;
	//FVector traceStart = GetCapsuleComponent()->GetComponentLocation() + scaledStartAndEnd;
	FVector traceStart = GetActorLocation() + scaledStartAndEnd;

	//FVector traceEnd = GetCapsuleComponent()->GetComponentLocation() - scaledStartAndEnd;
	FVector traceEnd = GetActorLocation() - scaledStartAndEnd;

	FQuat rotation = FQuat().Identity;
	//FQuat rotation = GetCapsuleComponent()->GetComponentQuat();
	//ECollisionChannel collisionChannel = ECollisionChannel::ECC_Pawn;
	ECollisionChannel collisionChannel = ECollisionChannel::ECC_WorldDynamic;
	FCollisionShape collisionShape = GetCapsuleComponent()->GetCollisionShape(5.0f);
	//FCollisionShape collisionShape = GetCapsuleComponent()->GetCollisionShape(10.0f);

	/*
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::_DoCapsuleCheck - scaledStartAndEnd = (%f, %f, %f)"), scaledStartAndEnd.X, scaledStartAndEnd.Y, scaledStartAndEnd.Z));
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::_DoCapsuleCheck - traceStart = (%f, %f, %f)"), traceStart.X, traceStart.Y, traceStart.Z));
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::_DoCapsuleCheck - traceEnd = (%f, %f, %f)"), traceEnd.X, traceEnd.Y, traceEnd.Z));
	*/

	bool isTouchingFloorBelow = false;
	bool isTouchingPieceAbove = false;

	if (GetWorld()->SweepMultiByChannel(outHits, traceStart, traceEnd, rotation, collisionChannel, collisionShape))
	{
		UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Capsule Sweep got some hits: %d"), outHits.Num());

		for (FHitResult hitResult : outHits)
		{
			hitResult.Actor->GetFullName().GetCharArray();
			UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Capsule Sweep Hit: %s"), *(hitResult.Actor->GetFullName()));

			if (hitResult.GetActor()->GetClass()->IsChildOf(ABoardPieceCPP::StaticClass()))
			{
				UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Board Piece found"));
				FVector bpLocation = hitResult.GetActor()->GetActorLocation();
				if (bpLocation.Z >= GetActorLocation().Z)
				{
					UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Board Piece found was above the fps player"));
					isTouchingPieceAbove = true;
				}
			}
			else if (hitResult.GetActor()->ActorHasTag("Floor"))
			{
				UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Floor found"));
				FVector bpLocation = hitResult.GetActor()->GetActorLocation();
				if (bpLocation.Z <= GetActorLocation().Z)
				{
					UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Floor found was below the fps player"));
					isTouchingFloorBelow = true;
				}
			}
			else if (hitResult.GetActor()->Tags.Contains("Floor"))
			{
				UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Floor found through alternate method"));
				FVector bpLocation = hitResult.GetActor()->GetActorLocation();
				if (bpLocation.Z <= GetActorLocation().Z)
				{
					UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - Floor found was below the fps player"));
					isTouchingFloorBelow = true;
				}
			}

			for (FName tag : hitResult.GetActor()->Tags) {
				UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - tag found = %s"), *(tag.ToString()));
			}

			UE_LOG(LogFPChar, Warning, TEXT("AF3MashUpCharacter::_DoCapsuleCheck - tag found? = %d"), hitResult.GetActor()->ActorHasTag("Floor"));
			
		}

		if (isTouchingPieceAbove && isTouchingFloorBelow)
		{
			ServerOnDamaged(100.0f);
		}
	}
}


void AF3MashUpCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AF3MashUpCharacter, Health);
	DOREPLIFETIME(AF3MashUpCharacter, CanFire);
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

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Pawn))
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

	FVector Start = GetActorLocation() + (ForwardVector * LineTraceStartDistance);
	FVector End = ((ForwardVector * LineTraceEndDistance) + Start);

	FHitResult OutHit;

	GetWorldTimerManager().SetTimer(CanFireTimer, this, &AF3MashUpCharacter::AllowFire, FireRate, false);

	if (GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Pawn))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::_OnFire - An object was detected")));
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, OutHit.GetActor()->GetFullName());

		if (OutHit.GetActor()->GetClass()->IsChildOf(AF3MashUpCharacter::StaticClass()))
		{
			AF3MashUpCharacter* other = Cast<AF3MashUpCharacter, AActor>(OutHit.GetActor());

			other->ServerOnDamaged(1.0f);
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("AF3MashUpCharacter::_OnFire - An object was not detected")));
	}
}

void AF3MashUpCharacter::AllowFire()
{
	CanFire = true;
}

void AF3MashUpCharacter::StartFiring()
{
	GetWorldTimerManager().SetTimer(KeepFiringTimer, this, &AF3MashUpCharacter::OnFire, FireRate, true);
}

void AF3MashUpCharacter::StopFiring()
{
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, "Timer should be cleared now");
	GetWorldTimerManager().ClearTimer(KeepFiringTimer);
}

void AF3MashUpCharacter::ServerOnDamaged_Implementation(float damage)
{
	_OnDamaged(damage);
}

void AF3MashUpCharacter::_OnDamaged(float damage)
{
	Health -= damage;

	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("An FPS Player's HP is: %f"), Health));
	//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, GetFullName());

	if (Health <= 0.0f)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("An FPS Player died")));
		//GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, GetFullName());

		Destroy();
	}
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

//Commenting this section out to be consistent with FPS BP template.
//This allows the user to turn without using the right virtual joystick

//void AF3MashUpCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
//{
//	if ((TouchItem.bIsPressed == true) && (TouchItem.FingerIndex == FingerIndex))
//	{
//		if (TouchItem.bIsPressed)
//		{
//			if (GetWorld() != nullptr)
//			{
//				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
//				if (ViewportClient != nullptr)
//				{
//					FVector MoveDelta = Location - TouchItem.Location;
//					FVector2D ScreenSize;
//					ViewportClient->GetViewportSize(ScreenSize);
//					FVector2D ScaledDelta = FVector2D(MoveDelta.X, MoveDelta.Y) / ScreenSize;
//					if (FMath::Abs(ScaledDelta.X) >= 4.0 / ScreenSize.X)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.X * BaseTurnRate;
//						AddControllerYawInput(Value);
//					}
//					if (FMath::Abs(ScaledDelta.Y) >= 4.0 / ScreenSize.Y)
//					{
//						TouchItem.bMoved = true;
//						float Value = ScaledDelta.Y * BaseTurnRate;
//						AddControllerPitchInput(Value);
//					}
//					TouchItem.Location = Location;
//				}
//				TouchItem.Location = Location;
//			}
//		}
//	}
//}

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