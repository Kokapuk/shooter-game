#include "SGPlayerState.h"

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
	DOREPLIFETIME_CONDITION(ASGPlayerState, Ability, COND_OwnerOnly);
	DOREPLIFETIME(ASGPlayerState, Kills);
	DOREPLIFETIME(ASGPlayerState, Deaths);
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

void ASGPlayerState::ServerSetAbility_Implementation(USGAbilityDataAsset* NewAbility)
{
	Ability = NewAbility;
}

void ASGPlayerState::AuthHandleKill()
{
	if (!HasAuthority()) return;

	Kills++;
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

	if (HasAuthority())
	{
		Deaths++;
	}
}
