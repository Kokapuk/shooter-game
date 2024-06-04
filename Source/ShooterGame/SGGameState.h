#pragma once

#include "CoreMinimal.h"
#include "Delegates.h"
#include "Enums.h"
#include "GameFramework/GameState.h"
#include "SGGameState.generated.h"

class ASGPlayerState;

UCLASS()
class SHOOTERGAME_API ASGGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnMatchBegin OnMatchBegin;

	UPROPERTY(BlueprintAssignable)
	FOnMatchFinish OnMatchFinish;

	UPROPERTY(BlueprintAssignable)
	FOnDie OnKill;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Set Match State")
	void AuthSetMatchState(const EMatchState NewMatchState);

	UFUNCTION(BlueprintPure)
	EMatchState GetShooterMatchState() const { return ShooterMatchState; }

protected:
	UPROPERTY(ReplicatedUsing=OnRep_ShooterMatchState, VisibleInstanceOnly)
	EMatchState ShooterMatchState;

	UFUNCTION(BlueprintPure)
	virtual FString GetMatchResult() const;

	UFUNCTION()
	void OnRep_ShooterMatchState();

	UFUNCTION()
	virtual void HandleMatchBegin();

	UFUNCTION()
	virtual void HandleKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot);
};
