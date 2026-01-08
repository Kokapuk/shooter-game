#include "SGPlayerState.h"

#include "SGCharacter.h"
#include "SGGameState.h"
#include "Kismet/GameplayStatics.h"
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
