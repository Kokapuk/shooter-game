#pragma once

#include "CoreMinimal.h"
#include "Delegates.h"
#include "GameFramework/PlayerState.h"
#include "SGPlayerState.generated.h"

class ASGCharacter;
class USGAbilityDataAsset;

UCLASS()
class SHOOTERGAME_API ASGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnDie OnDie;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure)
	ASGCharacter* GetCharacter() const { return Character; }

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Set Ability")
	void ServerSetAbility(USGAbilityDataAsset* NewAbility);

	UFUNCTION(BlueprintPure)
	USGAbilityDataAsset* GetAbility() const { return Ability; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="IncrementKills")
	void AuthIncrementKills();

	UFUNCTION(BlueprintPure)
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintPure)
	int32 GetDeaths() const { return Deaths; }

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly)
	ASGCharacter* Character;

	UPROPERTY(Replicated)
	USGAbilityDataAsset* Ability;

	UPROPERTY(Replicated)
	int32 Kills;

	UPROPERTY(Replicated)
	int32 Deaths;

	UFUNCTION()
	virtual void HandleMatchBegin();

	UFUNCTION()
	void HandleDie(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot);
};
