#include "SGPlayerState.h"

#include "SGCharacter.h"
#include "SGGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void ASGPlayerState::ServerRegisterPlayerInTeam_Implementation(const ETeam Team)
{
	ASGGameState* GameState = GetWorld()->GetGameState<ASGGameState>();
	check(IsValid(GameState))

	GameState->AuthRegisterPlayerInTeam(this, Team);
}

void ASGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGPlayerState, Character);
}

void ASGPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;
	
	ASGGameState* GameState = GetWorld()->GetGameState<ASGGameState>();
	check(IsValid(GameState))

	GameState->OnMatchBegin.AddUniqueDynamic(this, &ASGPlayerState::AuthHandleMatchBegin);
}

ETeam ASGPlayerState::GetTeam() const
{
	const ASGGameState* GameState = GetWorld()->GetGameState<ASGGameState>();
	check(IsValid(GameState))

	return GameState->GetPlayerTeam(this);
}

bool ASGPlayerState::IsDead() const
{
	if (!IsValid(Character)) return false;

	return Character->IsDead();
}

void ASGPlayerState::AuthHandleMatchBegin()
{
	if (!HasAuthority()) return;
	
	ASGCharacter* ControlledCharacter = GetPawn<ASGCharacter>();
	if (!IsValid(ControlledCharacter)) return;

	Character = ControlledCharacter;
}

void ASGPlayerState::MultiHandleDie_Implementation()
{
	OnDie.Broadcast();
}
