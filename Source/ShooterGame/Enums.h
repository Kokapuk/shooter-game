#pragma once

#include "Enums.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None,
	Red,
	Blue
};

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	WaitingToStart,
	InProgress,
	Finished
};

UENUM(BlueprintType)
enum class ERoundState : uint8
{
	None,
	InProgress,
	Finished
};