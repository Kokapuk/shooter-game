#pragma once

#include "CoreMinimal.h"
#include "ShooterGame/SGGameState.h"
#include "SGVersusGameState.generated.h"

class ASGVersusPlayerState;

UCLASS()
class SHOOTERGAME_API ASGVersusGameState : public ASGGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnRoundBegin OnRoundBegin;
	
	UPROPERTY(BlueprintAssignable)
	FOnRoundFinish OnRoundFinish;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Set Round State")
	void AuthSetRoundState(const ERoundState NewRoundState);

	UFUNCTION(BlueprintPure)
	int32 GetTeamSlotsNumber(const ETeam Team) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Register Player In Team")
	void AuthRegisterPlayerInTeam(ASGVersusPlayerState* Player, const ETeam Team);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Unregister Player From Team")
	void AuthUnregisterPlayerFromTeam(ASGVersusPlayerState* Player, const ETeam Team);

	UFUNCTION(BlueprintPure)
	TArray<ASGVersusPlayerState*> GetPlayersByTeam(const ETeam Team) const;

	UFUNCTION(BlueprintPure)
	TArray<ASGVersusPlayerState*> GetPlayers() const;

	UFUNCTION(BlueprintPure)
	ETeam GetPlayerTeam(const ASGVersusPlayerState* Player) const;

	UFUNCTION(BlueprintPure)
	int32 GetTeamScore(const ETeam Team) const;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_RoundState, VisibleInstanceOnly)
	ERoundState RoundState;

	UPROPERTY(VisibleInstanceOnly)
	int32 RedTeamSlotsNumber;
	
	UPROPERTY(VisibleInstanceOnly)
	int32 BlueTeamSlotsNumber;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<ASGVersusPlayerState*> RedTeamPlayers;
	
	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<ASGVersusPlayerState*> BlueTeamPlayers;
	
	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<ASGVersusPlayerState*> Spectators;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	int32 RedTeamScore;
	
	UPROPERTY(Replicated, VisibleInstanceOnly)
	int32 BlueTeamScore;

	virtual FString GetMatchResult() const override;
	virtual TArray<ASGPlayerState*> GetKillEventTargets() const override;
	virtual void HandleKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot) override;

	UFUNCTION()
	void OnRep_RoundState();
};
