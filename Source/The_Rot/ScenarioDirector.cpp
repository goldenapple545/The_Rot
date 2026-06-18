#include "ScenarioDirector.h"
#include "Engine/Engine.h"

AScenarioDirector::AScenarioDirector()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AScenarioDirector::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("ScenarioDirector: BeginPlay"));

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Green,
			TEXT("ScenarioDirector started")
		);
	}
}