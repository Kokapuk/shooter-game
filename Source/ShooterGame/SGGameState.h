#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SGGameState.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
	None,
	Red,
	Blue
};

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	WaitingToStart,
	InProgress
};

UENUM(BlueprintType)
enum class ERoundState : uint8
{
	None,
	InProgress,
	Finished
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchBegin);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundBegin);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundFinish);

UCLASS()
class SHOOTERGAME_API ASGGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnMatchBegin OnMatchBegin;

	UPROPERTY(BlueprintAssignable)
	FOnRoundBegin OnRoundBegin;

	UPROPERTY(BlueprintAssignable)
	FOnRoundFinish OnRoundFinish;

	virtual
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Set Match State")
	void AuthSetMatchState(const EMatchState NewMatchState);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Set Round State")
	void AuthSetRoundState(const ERoundState NewRoundState);

	UFUNCTION(BlueprintPure)
	EMatchState GetShooterMatchState() const { return ShooterMatchState; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Register Player In Team")
	void AuthRegisterPlayerInTeam(APlayerState* Player, const ETeam Team);

	UFUNCTION(BlueprintPure)
	TArray<APlayerState*> GetTeamPlayers(const ETeam Team) const;

	UFUNCTION(BlueprintPure)
	TArray<APlayerState*> GetAllPlayers() const;

	UFUNCTION(BlueprintPure)
	ETeam GetPlayerTeam(const APlayerState* Player) const;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_ShooterMatchState, VisibleInstanceOnly)
	EMatchState ShooterMatchState;

	UPROPERTY(ReplicatedUsing=OnRep_RoundState, VisibleInstanceOnly)
	ERoundState RoundState;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<APlayerState*> RedTeamPlayers;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<APlayerState*> BlueTeamPlayers;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<APlayerState*> Spectators;

private:
	UFUNCTION()
	void OnRep_ShooterMatchState();

	UFUNCTION()
	void OnRep_RoundState();

	UFUNCTION()
	void AuthHandleMatchBegin();

	UFUNCTION()
	void AuthHandlePlayerDie();
};
