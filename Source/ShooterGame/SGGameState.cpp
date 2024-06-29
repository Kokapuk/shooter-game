#include "SGGameState.h"

#include "EngineUtils.h"
#include "SGPlayerState.h"
#include "Net/UnrealNetwork.h"

void ASGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGGameState, ShooterMatchState);
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
		HandleMatchBegin();
		break;
	case EMatchState::Finished:
		OnMatchFinish.Broadcast(GetMatchResult());
		break;
	default:
		break;
	}
}

TArray<ASGPlayerState*> ASGGameState::GetKillEventTargets() const
{
	TArray<ASGPlayerState*> Targets;

	for (TActorIterator<ASGPlayerState> Target(GetWorld()); Target; ++Target)
	{
		if (Target->IsSpectator())
		{
			continue;
		}
		
		Targets.Add(*Target);
	}

	return Targets;
}

void ASGGameState::HandleMatchBegin()
{
	for (ASGPlayerState* Player : GetKillEventTargets())
	{
		Player->OnDie.AddUniqueDynamic(this, &ASGGameState::HandleKill);
	}
}

void ASGGameState::HandleKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot)
{
	OnKill.Broadcast(Killer, Victim, bIsHeadshot);
}
