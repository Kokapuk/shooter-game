#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SGPlayerState.generated.h"

class ASGCharacter;
enum class ETeam : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDie);

UCLASS()
class SHOOTERGAME_API ASGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnPlayerDie OnDie;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Register Player In Team")
	void ServerRegisterPlayerInTeam(const ETeam Team);

	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const;

	UFUNCTION(NetMulticast, Reliable)
	void MultiHandleDie();

	UFUNCTION(BlueprintPure)
	ASGCharacter* GetCharacter() const { return Character; }

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly)
	ASGCharacter* Character;

	UFUNCTION()
	void AuthHandleMatchBegin();
};
