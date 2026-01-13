#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SGGameMode.generated.h"

class ASGCharacter;
class USGAbilityDataAsset;
class USGWeaponDataAsset;
class USGWeaponComponent;
class ASGPlayerState;

UCLASS()
class SHOOTERGAME_API ASGGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASGGameMode();

	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;
	virtual bool MustSpectate_Implementation(APlayerController* NewPlayerController) const override;
	virtual void StartMatch() override;
	virtual void EndMatch() override;
	virtual bool ShouldTakeDamage(const ASGCharacter* Damager, const ASGCharacter* Target) const { return true; };

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<USGWeaponComponent> DefaultWeaponComponentClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USGWeaponDataAsset* DefaultWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USGAbilityDataAsset* DefaultAbility;

public:
	USGWeaponDataAsset* GetDefaultWeapon() const { return DefaultWeapon; }
	TSubclassOf<USGWeaponComponent> GetDefaultWeaponComponentClass() const { return DefaultWeaponComponentClass; }
	USGAbilityDataAsset* GetDefaultAbility() const { return DefaultAbility; }

	static void ShuffleActors(TArray<AActor*>& Actors);

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void ResetPlayer(ASGPlayerState* Player);

	UFUNCTION()
	virtual void HandlePlayerKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot);
};
