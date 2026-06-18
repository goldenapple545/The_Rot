#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ScenarioDirector.generated.h"

UCLASS()
class THE_ROT_API AScenarioDirector : public AActor
{
	GENERATED_BODY()

public:
	AScenarioDirector();

protected:
	virtual void BeginPlay() override;
};