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

void AF3MashUpCharacter::OnFire()
{
	ServerOnFire();
}

void AF3MashUpCharacter::OnRotateBoardPiece(float degrees)
{
	ServerDoRotateBoardPiece(degrees);
}

void AF3MashUpCharacter::ServerDoRotateBoardPiece_Implementation(float degrees)
{
	_DoRotateBoardPiece(degrees);
}

void AF3MashUpCharacter::_DoRotateBoardPiece(float degrees)
{
	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();
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

void AF3MashUpCharacter::ServerOnFire_Implementation() 
{
	_OnFire();
}

void AF3MashUpCharacter::_OnFire()
{
	if (!CanFire) {
		return;
	}

	CanFire = false;

	FVector ForwardVector = FirstPersonCameraComponent->GetForwardVector();
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
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, "Timer should be cleared now");
	GetWorldTimerManager().ClearTimer(KeepFiringTimer);
}

void AF3MashUpCharacter::ServerOnDamaged_Implementation(float damage)
{
	_OnDamaged(damage);
}

void AF3MashUpCharacter::_OnDamaged(float damage)
{
	Health -= damage;

	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("An FPS Player's HP is: %f"), Health));
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, GetFullName());

	if (Health <= 0.0f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("An FPS Player died")));
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, GetFullName());
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