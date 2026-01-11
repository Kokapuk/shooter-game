#pragma once

#include "CoreMinimal.h"
#include "SGCharacter.h"
#include "GameFramework/CheatManager.h"
#include "SGCheatManager.generated.h"

UCLASS()
class SHOOTERGAME_API USGCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void ApplyDamage(const float DamageAmount = 35.f);

	UFUNCTION(Server, Unreliable)
	void AuthApplyDamage(const float DamageAmount, ASGCharacter* Character);
};
