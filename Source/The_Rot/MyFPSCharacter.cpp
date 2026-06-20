#include "MyFPSCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

AMyFPSCharacter::AMyFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMyFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	FirstPersonCameraComponent = FindComponentByClass<UCameraComponent>();

	StandingCapsuleHalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();

	if (FirstPersonCameraComponent)
	{
		CameraLocationStanding = FirstPersonCameraComponent->GetRelativeLocation().Z;
		CameraLocationCurrent = CameraLocationStanding;
		CameraLocationTarget = CameraLocationStanding;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AMyFPSCharacter: CameraComponent not found."));
	}

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->NavAgentProps.bCanCrouch = true;
		MoveComp->CrouchedHalfHeight = CrouchedCapsuleHalfHeight;
		MoveComp->MaxWalkSpeed = StandingSpeed;
		MoveComp->MaxWalkSpeedCrouched = CrouchingSpeed;
	}
	UpdateMovementSpeedForCurrentStance();
}

void AMyFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!FirstPersonCameraComponent)
	{
		return;
	}

	CameraLocationCurrent = FMath::FInterpTo(
		CameraLocationCurrent,
		CameraLocationTarget,
		DeltaTime,
		CameraInterpSpeed
	);

	FVector RelLoc = FirstPersonCameraComponent->GetRelativeLocation();
	RelLoc.Z = CameraLocationCurrent;
	FirstPersonCameraComponent->SetRelativeLocation(RelLoc);
}

void AMyFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyFPSCharacter::OnStancePressed);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AMyFPSCharacter::OnStanceReleased);
}

void AMyFPSCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	CacheCurrentCameraZ();

	if (CurrentStance != EStanceState::Prone)
	{
		CurrentStance = EStanceState::Crouching;
		CameraLocationTarget = CameraLocationCrouching;
		UpdateMovementSpeedForCurrentStance();
	}
}

void AMyFPSCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	CacheCurrentCameraZ();

	if (CurrentStance != EStanceState::Prone)
	{
		CurrentStance = EStanceState::Standing;
		CameraLocationTarget = CameraLocationStanding;
		UpdateMovementSpeedForCurrentStance();
	}
}

void AMyFPSCharacter::OnStancePressed()
{
	bStanceButtonHeld = true;
	bHoldTriggered = false;
	StanceAtPress = CurrentStance;

	GetWorldTimerManager().ClearTimer(HoldToProneTimer);
	GetWorldTimerManager().SetTimer(
		HoldToProneTimer,
		this,
		&AMyFPSCharacter::HandleHoldToProne,
		HoldToProneTime,
		false
	);

	if (CurrentStance == EStanceState::Standing)
	{
		Crouch();
	}
}

void AMyFPSCharacter::OnStanceReleased()
{
	bStanceButtonHeld = false;
	GetWorldTimerManager().ClearTimer(HoldToProneTimer);

	const FString StanceString = StaticEnum<EStanceState>()->GetNameStringByValue(static_cast<int64>(StanceAtPress));
	
	UE_LOG(LogTemp, Warning, TEXT("AMyFPSCharacter: %s"), *StanceString);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, StanceString);
	}
	
	if (bHoldTriggered)
	{
		if (CurrentStance == EStanceState::Prone)
		{
			TryProneToCrouch();
		}
		return;
	}

	switch (StanceAtPress)
	{
		case EStanceState::Standing:
			break;

		case EStanceState::Crouching:
			UnCrouch();
			break;

		case EStanceState::Prone:
			TryProneToCrouch();
			break;

		default:
			break;
	}
}

void AMyFPSCharacter::HandleHoldToProne()
{
	if (!bStanceButtonHeld)
	{
		return;
	}

	bHoldTriggered = true;

	if (CurrentStance != EStanceState::Prone)
	{
		EnterProne();
	}
}

void AMyFPSCharacter::EnterProne()
{
	CacheCurrentCameraZ();

	GetWorldTimerManager().ClearTimer(HoldToProneTimer);

	GetCapsuleComponent()->SetCapsuleHalfHeight(ProneCapsuleHalfHeight, true);

	CurrentStance = EStanceState::Prone;
	CameraLocationTarget = CameraLocationLying;
	UpdateMovementSpeedForCurrentStance();
}

void AMyFPSCharacter::TryProneToCrouch()
{
	if (!CanChangeCapsuleHalfHeight(CrouchedCapsuleHalfHeight))
	{
		const FString StanceString = "AMyFPSCharacter: Cant prone to couch.";
		UE_LOG(LogTemp, Warning, TEXT("AMyFPSCharacter: %s"), *StanceString);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, StanceString);
		}
		return;
	}

	CacheCurrentCameraZ();

	GetCapsuleComponent()->SetCapsuleHalfHeight(CrouchedCapsuleHalfHeight, true);

	CurrentStance = EStanceState::Crouching;
	CameraLocationTarget = CameraLocationCrouching;
	UpdateMovementSpeedForCurrentStance();
}

void AMyFPSCharacter::UpdateCameraTargetForCurrentStance()
{
	switch (CurrentStance)
	{
	case EStanceState::Standing:
		CameraLocationTarget = CameraLocationStanding;
		break;

	case EStanceState::Crouching:
		CameraLocationTarget = CameraLocationCrouching;
		break;

	case EStanceState::Prone:
		CameraLocationTarget = CameraLocationLying;
		break;

	default:
		break;
	}
}

void AMyFPSCharacter::CacheCurrentCameraZ()
{
	if (!FirstPersonCameraComponent)
	{
		return;
	}

	CameraLocationCurrent = FirstPersonCameraComponent->GetRelativeLocation().Z;
}

void AMyFPSCharacter::UpdateMovementSpeedForCurrentStance()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	switch (CurrentStance)
	{
	case EStanceState::Standing:
		MoveComp->MaxWalkSpeed = StandingSpeed;
		break;

	case EStanceState::Crouching:
		MoveComp->MaxWalkSpeedCrouched = CrouchingSpeed;
		break;

	case EStanceState::Prone:
		MoveComp->MaxWalkSpeedCrouched = ProneSpeed;
		break;

	default:
		break;
	}
}

bool AMyFPSCharacter::CanChangeCapsuleHalfHeight(float TargetHalfHeight) const
{
	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	const UWorld* World = GetWorld();
	if (!Capsule || !World)
	{
		return false;
	}

	const float CurrentRadius = Capsule->GetUnscaledCapsuleRadius();
	const float CurrentHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
	const float HalfHeightDelta = TargetHalfHeight - CurrentHalfHeight;

	FVector TestLocation = GetActorLocation();
	TestLocation.Z += HalfHeightDelta;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(CanChangeCapsuleHalfHeight), false, this);

	const bool bBlocked = World->OverlapBlockingTestByChannel(
		TestLocation,
		GetActorQuat(),
		ECC_Visibility,
		FCollisionShape::MakeCapsule(CurrentRadius, TargetHalfHeight),
		Params
	);

	return !bBlocked;
}