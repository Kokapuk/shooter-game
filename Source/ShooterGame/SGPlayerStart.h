#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SGPlayerStart.generated.h"

UCLASS()
class SHOOTERGAME_API ASGPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ASGPlayerStart(const FObjectInitializer& ObjectInitializer);
};
