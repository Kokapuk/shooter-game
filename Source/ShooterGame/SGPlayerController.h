#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SGPlayerController.generated.h"

UCLASS()
class SHOOTERGAME_API ASGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void AddYawInput(float Value) override;
	virtual void AddPitchInput(float Value) override;

private:
	float Sensitivity = .6f;
};
