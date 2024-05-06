#include "SGGameState.h"

#include "SGGameMode.h"
#include "SGPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ASGGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGGameState, ShooterMatchState);
	DOREPLIFETIME(ASGGameState, RedTeamPlayers);
	DOREPLIFETIME(ASGGameState, BlueTeamPlayers);
	DOREPLIFETIME(ASGGameState, Spectators);
}

void ASGGameState::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		OnMatchBegin.AddDynamic(this, &ASGGameState::AuthHandleMatchBegin);
	}
}

void ASGGameState::AuthSetMatchState(const EMatchState NewMatchState)
{
	if (!HasAuthority()) return;

	ShooterMatchState = NewMatchState;
	OnRep_ShooterMatchState();
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

void ASGGameState::AuthRegisterPlayerInTeam(APlayerState* Player, const ETeam Team)
{
	if (!HasAuthority()) return;

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
	}
}

void ASGGameState::AuthHandleMatchBegin()
{
	if (!HasAuthority()) return;

	for (int32 i = 0; i < RedTeamPlayers.Num(); ++i)
	{
		ASGPlayerState* PlayerState = Cast<ASGPlayerState>(RedTeamPlayers[i]);
		PlayerState->OnDie.AddUniqueDynamic(this, &ASGGameState::AuthHandlePlayerDie);
	}

	for (int32 i = 0; i < BlueTeamPlayers.Num(); ++i)
	{
		ASGPlayerState* PlayerState = Cast<ASGPlayerState>(BlueTeamPlayers[i]);
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

	const UWorld* World = GetWorld();
	if (!IsValid(World)) return;
	
	ASGGameMode* GameMode = World->GetAuthGameMode<ASGGameMode>();
	if (!IsValid(GameMode)) return;
	
	GameMode->ResetPlayers();
}
