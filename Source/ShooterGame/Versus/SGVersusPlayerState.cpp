#include "SGVersusPlayerState.h"

#include "SGVersusGameState.h"

void ASGVersusPlayerState::ServerRegisterPlayerInTeam_Implementation(const ETeam Team)
{
	ASGVersusGameState* GameState = GetWorld()->GetGameState<ASGVersusGameState>();
	check(IsValid(GameState))

	GameState->AuthRegisterPlayerInTeam(this, Team);
}

void ASGVersusPlayerState::ServerUnregisterPlayerFromTeam_Implementation(const ETeam Team)
{
	ASGVersusGameState* GameState = GetWorld()->GetGameState<ASGVersusGameState>();
	check(IsValid(GameState))

	GameState->AuthUnregisterPlayerFromTeam(this, Team);
}

ETeam ASGVersusPlayerState::GetTeam() const
{
	const ASGVersusGameState* GameState = GetWorld()->GetGameState<ASGVersusGameState>();
	check(IsValid(GameState))

	return GameState->GetPlayerTeam(this);
}

void ASGVersusPlayerState::HandleMatchBegin()
{
	Super::HandleMatchBegin();

	if (HasAuthority())
	{
		SetIsSpectator(GetTeam() == ETeam::None);
	}
}
