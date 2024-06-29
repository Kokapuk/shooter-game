#pragma once

#include "CoreMinimal.h"
#include "ShooterGame/SGCharacter.h"
#include "SGRouletteCharacter.generated.h"

UCLASS()
class SHOOTERGAME_API ASGRouletteCharacter : public ASGCharacter
{
	GENERATED_BODY()

public:
	ASGRouletteCharacter(const FObjectInitializer& ObjectInitializer);
};
