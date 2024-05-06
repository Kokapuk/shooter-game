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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchBegin);

UCLASS()
class SHOOTERGAME_API ASGGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnMatchBegin OnMatchBegin;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Set Match State")
	void AuthSetMatchState(const EMatchState NewMatchState);

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
	void AuthHandleMatchBegin();

	UFUNCTION()
	void AuthHandlePlayerDie();
};
