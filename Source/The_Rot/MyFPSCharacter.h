#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyFPSCharacter.generated.h"

class UCameraComponent;

UENUM(BlueprintType)
enum class EStanceState : uint8
{
	Standing	UMETA(DisplayName = "Standing"),
	Crouching	UMETA(DisplayName = "Crouching"),
	Prone		UMETA(DisplayName = "Prone")
};

UCLASS()
class THE_ROT_API AMyFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMyFPSCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance")
	float HoldToProneTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance")
	float CameraInterpSpeed = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance")
	float CameraLocationStanding = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance")
	float CameraLocationCrouching = -60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance")
	float CameraLocationLying = -150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance")
	float ProneCapsuleHalfHeight = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stance")
	float CrouchedCapsuleHalfHeight = 44.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stance")
	EStanceState CurrentStance = EStanceState::Standing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stance")
	float CameraLocationCurrent = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stance")
	float CameraLocationTarget = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float StandingSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchingSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ProneSpeed = 120.0f;

private:
	UPROPERTY()
	UCameraComponent* FirstPersonCameraComponent = nullptr;

	float StandingCapsuleHalfHeight = 0.0f;
	bool bStanceButtonHeld = false;
	bool bHoldTriggered = false;
	EStanceState StanceAtPress = EStanceState::Standing;

	FTimerHandle HoldToProneTimer;

	void OnStancePressed();
	void OnStanceReleased();
	void HandleHoldToProne();

	void EnterProne();
	void TryProneToCrouch();
	void UpdateCameraTargetForCurrentStance();
	void CacheCurrentCameraZ();

	void UpdateMovementSpeedForCurrentStance();

	bool CanChangeCapsuleHalfHeight(float TargetHalfHeight) const;
};