#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "SGCheatManager.generated.h"

UCLASS()
class SHOOTERGAME_API USGCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void ApplyDamage(const float DamageAmount = 35.f) { ServerApplyDamage(DamageAmount); }

	UFUNCTION(Exec)
	void SetTimeScale(const float NewTimeScale) { ServerSetTimeScale(NewTimeScale); }

protected:
	UFUNCTION(Server, Unreliable)
	void ServerApplyDamage(const float DamageAmount);

	UFUNCTION(Server, Unreliable)
	void ServerSetTimeScale(const float NewTimeScale);

	UFUNCTION(NetMulticast, Reliable)
	void MultiSetTimeScale(const float NewTimeScale);
};
