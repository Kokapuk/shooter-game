#pragma once

#include "CoreMinimal.h"
#include "ShooterGame/SGPlayerState.h"
#include "SGVersusPlayerState.generated.h"

UCLASS()
class SHOOTERGAME_API ASGVersusPlayerState : public ASGPlayerState
{
	GENERATED_BODY()

public:
	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Register Player In Team")
	void ServerRegisterPlayerInTeam(const ETeam Team);

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Unregister Player From Team")
	void ServerUnregisterPlayerFromTeam(const ETeam Team);

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const;

protected:
	virtual void HandleMatchBegin() override;
};
