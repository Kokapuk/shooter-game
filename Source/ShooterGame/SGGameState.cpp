#include "SGGameState.h"

#include "EngineUtils.h"
#include "SGGameMode.h"
#include "SGPlayerStart.h"
#include "SGPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ASGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGGameState, ShooterMatchState);
	DOREPLIFETIME(ASGGameState, RoundState);
	DOREPLIFETIME(ASGGameState, RedTeamPlayers);
	DOREPLIFETIME(ASGGameState, BlueTeamPlayers);
	DOREPLIFETIME(ASGGameState, Spectators);
	DOREPLIFETIME(ASGGameState, RedTeamScore);
	DOREPLIFETIME(ASGGameState, BlueTeamScore);
}

void ASGGameState::BeginPlay()
{
	Super::BeginPlay();

	for (TActorIterator<ASGPlayerStart> Iterator(GetWorld()); Iterator; ++Iterator)
	{
		switch (Iterator->GetTeam())
		{
		case ETeam::Red:
			RedTeamSlotsNumber++;
			break;
		case ETeam::Blue:
			BlueTeamSlotsNumber++;
			break;
		}
	}

	if (!HasAuthority()) return;
	OnMatchBegin.AddDynamic(this, &ASGGameState::AuthHandleMatchBegin);
}

void ASGGameState::AuthSetMatchState(const EMatchState NewMatchState)
{
	if (!HasAuthority()) return;

	ShooterMatchState = NewMatchState;
	OnRep_ShooterMatchState();
}

void ASGGameState::AuthSetRoundState(const ERoundState NewRoundState)
{
	if (!HasAuthority()) return;

	RoundState = NewRoundState;
	OnRep_RoundState();
}

TArray<APlayerState*> ASGGameState::GetTeamPlayers(const ETeam Team) const
{
	switch (Team)
	{
	case ETeam::Red: return RedTeamPlayers;
	case ETeam::Blue: return BlueTeamPlayers;
	case ETeam::None: return Spectators;
	default: return {};
	}
}

TArray<APlayerState*> ASGGameState::GetAllPlayers() const
{
	TArray<APlayerState*> AllPlayers = {};

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

ETeam ASGGameState::GetPlayerTeam(const APlayerState* Player) const
{
	if (RedTeamPlayers.Contains(Player)) return ETeam::Red;
	if (BlueTeamPlayers.Contains(Player)) return ETeam::Blue;

	return ETeam::None;
}

int32 ASGGameState::GetTeamScore(const ETeam Team) const
{
	switch (Team)
	{
	case ETeam::Red:
		return RedTeamScore;
	case ETeam::Blue:
		return BlueTeamScore;
	default:
		return -1;
	}
}

int32 ASGGameState::GetTeamSlotsNumber(const ETeam Team) const
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

void ASGGameState::AuthRegisterPlayerInTeam(APlayerState* Player, const ETeam Team)
{
	if (!HasAuthority() || GetTeamSlotsNumber(Team) == GetTeamPlayers(Team).Num())
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

void ASGGameState::OnRep_ShooterMatchState()
{
	switch (ShooterMatchState)
	{
	case EMatchState::InProgress:
		OnMatchBegin.Broadcast();
		break;
	case EMatchState::Finished:
		OnMatchFinish.Broadcast(RedTeamScore > BlueTeamScore
			                        ? ETeam::Red
			                        : BlueTeamScore > RedTeamScore
			                        ? ETeam::Blue
			                        : ETeam::None);
		break;
	}
}

void ASGGameState::OnRep_RoundState()
{
	switch (RoundState)
	{
	case ERoundState::InProgress:
		OnRoundBegin.Broadcast();
		break;
	case ERoundState::Finished:
		OnRoundFinish.Broadcast();
		break;
	}
}

void ASGGameState::AuthHandleMatchBegin()
{
	if (!HasAuthority()) return;

	TArray<APlayerState*> AllPlayers = GetAllPlayers();

	for (int32 i = 0; i < AllPlayers.Num(); ++i)
	{
		ASGPlayerState* PlayerState = Cast<ASGPlayerState>(AllPlayers[i]);
		check(PlayerState)

		PlayerState->OnDie.AddUniqueDynamic(this, &ASGGameState::AuthHandlePlayerDie);
	}
}

void ASGGameState::AuthHandlePlayerDie()
{
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

	ASGGameMode* GameMode = GetWorld()->GetAuthGameMode<ASGGameMode>();
	check(IsValid(GameMode))

	GameMode->FinishRound();
}
