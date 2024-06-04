#include "SGVersusGameState.h"

#include "EngineUtils.h"
#include "SGVersusGameMode.h"
#include "SGVersusPlayerStart.h"
#include "SGVersusPlayerState.h"
#include "Net/UnrealNetwork.h"

void ASGVersusGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGVersusGameState, RoundState);
	DOREPLIFETIME(ASGVersusGameState, RedTeamPlayers);
	DOREPLIFETIME(ASGVersusGameState, BlueTeamPlayers);
	DOREPLIFETIME(ASGVersusGameState, Spectators);
	DOREPLIFETIME(ASGVersusGameState, RedTeamScore);
	DOREPLIFETIME(ASGVersusGameState, BlueTeamScore);
}

void ASGVersusGameState::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ASGVersusPlayerStart> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		switch (Iterator->GetTeam())
		{
		case ETeam::Red:
			RedTeamSlotsNumber++;
			break;
		case ETeam::Blue:
			BlueTeamSlotsNumber++;
			break;
		default:
			break;
		}
	}
}

void ASGVersusGameState::AuthSetRoundState(const ERoundState NewRoundState)
{
	if (!HasAuthority())
	{
		return;
	}

	RoundState = NewRoundState;
	OnRep_RoundState();
}

TArray<ASGVersusPlayerState*> ASGVersusGameState::GetPlayersByTeam(const ETeam Team) const
{
	switch (Team)
	{
	case ETeam::Red: return RedTeamPlayers;
	case ETeam::Blue: return BlueTeamPlayers;
	case ETeam::None: return Spectators;
	default: return {};
	}
}

TArray<ASGVersusPlayerState*> ASGVersusGameState::GetPlayers() const
{
	TArray<ASGVersusPlayerState*> AllPlayers = {};

	for (int32 i = 0; i < RedTeamPlayers.Num(); ++i)
	{
		AllPlayers.Add(RedTeamPlayers[i]);
	}

	for (int32 i = 0; i < BlueTeamPlayers.Num(); ++i)
	{
		AllPlayers.Add(BlueTeamPlayers[i]);
	}

	return AllPlayers;
}

ETeam ASGVersusGameState::GetPlayerTeam(const ASGVersusPlayerState* Player) const
{
	if (RedTeamPlayers.Contains(Player)) return ETeam::Red;
	if (BlueTeamPlayers.Contains(Player)) return ETeam::Blue;

	return ETeam::None;
}

int32 ASGVersusGameState::GetTeamScore(const ETeam Team) const
{
	switch (Team)
	{
	case ETeam::Red:
		return RedTeamScore;
	case ETeam::Blue:
		return BlueTeamScore;
	default:
		return 0;
	}
}

int32 ASGVersusGameState::GetTeamSlotsNumber(const ETeam Team) const
{
	switch (Team)
	{
	case ETeam::Red:
		return RedTeamSlotsNumber;
	case ETeam::Blue:
		return BlueTeamSlotsNumber;
	default:
		return -1;
	}
}

void ASGVersusGameState::AuthRegisterPlayerInTeam(ASGVersusPlayerState* Player, const ETeam Team)
{
	if (!HasAuthority() || GetTeamSlotsNumber(Team) == GetPlayersByTeam(Team).Num())
	{
		return;
	}

	switch (Team)
	{
	case ETeam::Red:
		BlueTeamPlayers.Contains(Player) && BlueTeamPlayers.Remove(Player);
		Spectators.Contains(Player) && Spectators.Remove(Player);

		if (RedTeamPlayers.Contains(Player)) return;
		RedTeamPlayers.Add(Player);

		break;
	case ETeam::Blue:
		RedTeamPlayers.Contains(Player) && RedTeamPlayers.Remove(Player);
		Spectators.Contains(Player) && Spectators.Remove(Player);

		if (BlueTeamPlayers.Contains(Player)) return;
		BlueTeamPlayers.Add(Player);

		break;
	case ETeam::None:
		RedTeamPlayers.Contains(Player) && RedTeamPlayers.Remove(Player);
		BlueTeamPlayers.Contains(Player) && BlueTeamPlayers.Remove(Player);

		if (Spectators.Contains(Player)) return;
		Spectators.Add(Player);

		break;
	}
}

void ASGVersusGameState::AuthUnregisterPlayerFromTeam(ASGVersusPlayerState* Player, const ETeam Team)
{
	if (!HasAuthority()) return;

	switch (Team)
	{
	case ETeam::Red:
		RedTeamPlayers.Remove(Player);
	case ETeam::Blue:
		BlueTeamPlayers.Remove(Player);
	case ETeam::None:
		Spectators.Remove(Player);
	}
}

FString ASGVersusGameState::GetMatchResult() const
{
	if (RedTeamScore == BlueTeamScore)
	{
		return "Tie";
	}

	return FString::Printf(TEXT("%hs team has won"), RedTeamScore > BlueTeamScore ? "Red" : "Blue");
}

void ASGVersusGameState::HandleMatchBegin()
{
	Super::HandleMatchBegin();

	for (ASGVersusPlayerState* Player : GetPlayers())
	{
		Player->OnDie.AddUniqueDynamic(this, &ASGVersusGameState::HandleKill);
	}
}

void ASGVersusGameState::HandleKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot)
{
	Super::HandleKill(Killer, Victim, bIsHeadshot);

	if (!HasAuthority()) return;

	bool bAreAllRedTeamMembersDead = true;
	bool bAreAllBlueTeamMembersDead = true;

	for (int32 i = 0; i < RedTeamPlayers.Num(); ++i)
	{
		const ASGPlayerState* PlayerState = Cast<ASGPlayerState>(RedTeamPlayers[i]);
		if (PlayerState->IsDead()) continue;

		bAreAllRedTeamMembersDead = false;
		break;
	}

	for (int32 i = 0; i < BlueTeamPlayers.Num(); ++i)
	{
		const ASGPlayerState* PlayerState = Cast<ASGPlayerState>(BlueTeamPlayers[i]);
		if (PlayerState->IsDead()) continue;

		bAreAllBlueTeamMembersDead = false;
		break;
	}

	if (!bAreAllRedTeamMembersDead && !bAreAllBlueTeamMembersDead) return;

	if (bAreAllRedTeamMembersDead) BlueTeamScore++;
	else if (bAreAllBlueTeamMembersDead) RedTeamScore++;

	ASGVersusGameMode* GameMode = GetWorld()->GetAuthGameMode<ASGVersusGameMode>();
	check(IsValid(GameMode))

	GameMode->FinishRound();
}

void ASGVersusGameState::OnRep_RoundState()
{
	switch (RoundState)
	{
	case ERoundState::InProgress:
		OnRoundBegin.Broadcast();
		break;
	case ERoundState::Finished:
		OnRoundFinish.Broadcast();
		break;
	default:
		break;
	}
}
