#include "SGPlayerState.h"

#include "SGGameState.h"
#include "Kismet/GameplayStatics.h"

void ASGPlayerState::ServerRegisterPlayerInTeam_Implementation(const ETeam Team)
{
	const UWorld* World = GetWorld();
	if (!IsValid(World)) return;

	ASGGameState* GameState = World->GetGameState<ASGGameState>();
	if (!IsValid(GameState)) return;

	GameState->AuthRegisterPlayerInTeam(this, Team);
}

ETeam ASGPlayerState::GetTeam() const
{
	const UWorld* World = GetWorld();
	if (!IsValid(World)) return ETeam::None;

	const ASGGameState* GameState = World->GetGameState<ASGGameState>();
	if (!IsValid(GameState)) return ETeam::None;

	return GameState->GetPlayerTeam(this);
}

void ASGPlayerState::AuthReset()
{
	if (!HasAuthority()) return;
	
	bIsDead = false;
}

void ASGPlayerState::MultiHandleDie_Implementation()
{
	bIsDead = true;
	OnDie.Broadcast();
}
