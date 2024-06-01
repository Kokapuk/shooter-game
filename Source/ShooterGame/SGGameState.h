#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameFramework/PlayerState.h"
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
	InProgress,
	Finished
};

UENUM(BlueprintType)
enum class ERoundState : uint8
{
	None,
	InProgress,
	Finished
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMatchBegin);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchFinish, ETeam, WinnerTeam);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundBegin);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundFinish);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnKill, APlayerState*, Killer, APlayerState*, Victim, bool, bIsHeadshot); 

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
	FOnRoundBegin OnRoundBegin;

	UPROPERTY(BlueprintAssignable)
	FOnRoundFinish OnRoundFinish;

	UPROPERTY(BlueprintAssignable)
	FOnKill OnKill;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Set Match State")
	void AuthSetMatchState(const EMatchState NewMatchState);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Set Round State")
	void AuthSetRoundState(const ERoundState NewRoundState);

	UFUNCTION(BlueprintPure)
	EMatchState GetShooterMatchState() const { return ShooterMatchState; }

	UFUNCTION(BlueprintPure)
	int32 GetTeamSlotsNumber(const ETeam Team) const;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Register Player In Team")
	void AuthRegisterPlayerInTeam(APlayerState* Player, const ETeam Team);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Unregister Player From Team")
	void AuthUnregisterPlayerFromTeam(APlayerState* Player, const ETeam Team);

	UFUNCTION(NetMulticast, Reliable)
	void MultiHandleKill(APlayerState* Killer, APlayerState* Victim, bool bIsHeadshot);

	UFUNCTION(BlueprintPure)
	TArray<APlayerState*> GetPlayersByTeam(const ETeam Team) const;

	UFUNCTION(BlueprintPure)
	TArray<APlayerState*> GetPlayers() const;

	UFUNCTION(BlueprintPure)
	ETeam GetPlayerTeam(const APlayerState* Player) const;

	UFUNCTION(BlueprintPure)
	int32 GetTeamScore(const ETeam Team) const;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_ShooterMatchState, VisibleInstanceOnly)
	EMatchState ShooterMatchState;

	UPROPERTY(ReplicatedUsing=OnRep_RoundState, VisibleInstanceOnly)
	ERoundState RoundState;

	UPROPERTY(VisibleInstanceOnly)
	int32 RedTeamSlotsNumber;

	UPROPERTY(VisibleInstanceOnly)
	int32 BlueTeamSlotsNumber;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<APlayerState*> RedTeamPlayers;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<APlayerState*> BlueTeamPlayers;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	TArray<APlayerState*> Spectators;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	int32 RedTeamScore;

	UPROPERTY(Replicated, VisibleInstanceOnly)
	int32 BlueTeamScore;

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
