#include "SGGameState.h"

#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"

void ASGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGGameState, ShooterMatchState);
}

void ASGGameState::BeginPlay()
{
	Super::BeginPlay();

	OnMatchBegin.AddDynamic(this, &ASGGameState::HandleMatchBegin);
}

void ASGGameState::AuthSetMatchState(const EMatchState NewMatchState)
{
	if (!HasAuthority())
	{
		return;
	}

	ShooterMatchState = NewMatchState;
	OnRep_ShooterMatchState();
}

FString ASGGameState::GetMatchResult() const
{
	return "Match has ended";
}

void ASGGameState::OnRep_ShooterMatchState()
{
	switch (ShooterMatchState)
	{
	case EMatchState::InProgress:
		OnMatchBegin.Broadcast();
		break;
	case EMatchState::Finished:
		OnMatchFinish.Broadcast(GetMatchResult());
		break;
	default:
		break;
	}
}

void ASGGameState::HandleMatchBegin()
{
}

void ASGGameState::HandleKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot)
{
	OnKill.Broadcast(Killer, Victim, bIsHeadshot);
}
