#pragma once

#include "CoreMinimal.h"
#include "ShooterGame/SGGameMode.h"
#include "SGRouletteGameMode.generated.h"

class ASGCharacter;

UCLASS()
class SHOOTERGAME_API ASGRouletteGameMode : public ASGGameMode
{
	GENERATED_BODY()

public:
	ASGRouletteGameMode();

	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
	virtual void StartMatch() override;

protected:
	UPROPERTY(EditDefaultsOnly)
	float RespawnTime;
	
	virtual void HandlePlayerKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot) override;

	UFUNCTION()
	void GiveBulletToRandomCharacter();
};
