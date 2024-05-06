#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SGPlayerState.generated.h"

enum class ETeam : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDie);

UCLASS()
class SHOOTERGAME_API ASGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnDie OnDie;

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Register Player In Team")
	void ServerRegisterPlayerInTeam(const ETeam Team);

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const;

	UFUNCTION(NetMulticast, Reliable)
	void MultiHandleDie();

	UFUNCTION(BlueprintPure)
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Reset")
	void AuthReset();

protected:
	uint8 bIsDead : 1;
};
