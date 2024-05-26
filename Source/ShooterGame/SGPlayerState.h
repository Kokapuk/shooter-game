#pragma once

#include "CoreMinimal.h"
#include "SGAbilityDataAsset.h"
#include "SGCharacter.h"
#include "GameFramework/PlayerState.h"
#include "SGPlayerState.generated.h"

class USGAbilityDataAsset;
enum class ETeam : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDie);

UCLASS()
class SHOOTERGAME_API ASGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnDie OnDie;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Register Player In Team")
	void ServerRegisterPlayerInTeam(const ETeam Team);

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const;

	UFUNCTION(BlueprintPure)
	ASGCharacter* GetCharacter() const { return Character; }

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Set Ability")
	void ServerSetAbility(USGAbilityDataAsset* NewAbility);

	UFUNCTION(BlueprintPure)
	USGAbilityDataAsset* GetAbility() const { return Ability; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const { return IsValid(Character) ? Character->IsDead() : false; }

	UFUNCTION(BlueprintPure)
	int32 GetKills() const { return Kills; }

	UFUNCTION(BlueprintPure)
	int32 GetDeaths() const { return Deaths; }

	UFUNCTION(NetMulticast, Reliable)
	void MultiHandleDie();

	void AuthHandleKill();

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
	void AuthHandleMatchBegin();
};
