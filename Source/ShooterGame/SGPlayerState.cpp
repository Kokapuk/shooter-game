#include "SGPlayerState.h"

#include "SGCharacter.h"
#include "SGGameState.h"
#include "Net/UnrealNetwork.h"

void ASGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGPlayerState, Character);
	DOREPLIFETIME(ASGPlayerState, Ability);
	DOREPLIFETIME(ASGPlayerState, Kills);
	DOREPLIFETIME(ASGPlayerState, Deaths);
}

void ASGPlayerState::BeginPlay()
{
	Super::BeginPlay();

	ASGGameState* GameState = GetWorld()->GetGameState<ASGGameState>();
	check(IsValid(GameState))

	GameState->OnMatchBegin.AddUniqueDynamic(this, &ASGPlayerState::HandleMatchBegin);

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(RTTCalculationTimerHandle, this, &ASGPlayerState::AuthUpdateRoundTripTime, 0.1f,
		                                true, 0.f);
	}
}

void ASGPlayerState::ServerSetAbility_Implementation(USGAbilityDataAsset* NewAbility)
{
	Ability = NewAbility;
}

bool ASGPlayerState::IsDead() const
{
	return IsValid(Character) ? Character->IsDead() : false;
}

void ASGPlayerState::AuthIncrementKills()
{
	if (!HasAuthority())
	{
		return;
	}

	Kills++;
}

void ASGPlayerState::HandleMatchBegin()
{
	Character = GetPawn<ASGCharacter>();

	if (IsValid(Character))
	{
		Character->OnDie.AddUniqueDynamic(this, &ASGPlayerState::HandleDie);
	}
}

void ASGPlayerState::HandleDie(ASGPlayerState* Killer, ASGPlayerState* Victim, const bool bIsHeadshot)
{
	OnDie.Broadcast(Killer, Victim, bIsHeadshot);

	if (HasAuthority())
	{
		Killer->AuthIncrementKills();

		SetIsSpectator(true);
		Deaths++;
	}
}

void ASGPlayerState::AuthUpdateRoundTripTime()
{
	if (!HasAuthority()) return;
	ClientUpdateRoundTripTime(GetWorld()->GetTimeSeconds());
}

void ASGPlayerState::ClientUpdateRoundTripTime_Implementation(const float Timestamp)
{
	ServerUpdateRoundTripTime(Timestamp);
}

void ASGPlayerState::ServerUpdateRoundTripTime_Implementation(const float Timestamp)
{
	RoundTripTime = GetWorld()->GetTimeSeconds() - Timestamp;
}
