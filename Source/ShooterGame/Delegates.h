#pragma once

#include "Delegates.Generated.h"

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchBegin);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFinish, FString, MatchResult);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundBegin);

UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundFinish);


UDELEGATE()
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDie, ASGPlayerState*, Killer, ASGPlayerState*, Victim, bool,
                                               bIsHeadhshot);
