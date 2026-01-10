#pragma once

#include "CoreMinimal.h"
#include "SGCharacter.h"
#include "SGWeaponDataAsset.h"
#include "GameFramework/GameMode.h"
#include "SGGameMode.generated.h"

class ASGPlayerState;

UCLASS()
class SHOOTERGAME_API ASGGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASGGameMode();

	virtual bool MustSpectate_Implementation(APlayerController* NewPlayerController) const override;
	virtual void StartMatch() override;
	virtual void EndMatch() override;
	virtual bool ShouldTakeDamage(const ASGCharacter* Damager, const ASGCharacter* Target) const { return true; };

protected:
	UPROPERTY(EditDefaultsOnly)
	USGWeaponDataAsset* DefaultWeapon;

public:
	USGWeaponDataAsset* GetDefaultWeapon() const { return DefaultWeapon; }

	static void ShuffleActors(TArray<AActor*>& Actors);

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void ResetPlayer(ASGPlayerState* Player);

	UFUNCTION()
	virtual void HandlePlayerKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot);
};
