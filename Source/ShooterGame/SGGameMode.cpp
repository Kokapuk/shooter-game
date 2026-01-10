#include "SGGameMode.h"

#include "SGCharacter.h"
#include "SGGameInstance.h"
#include "SGGameState.h"
#include "SGPlayerController.h"
#include "SGPlayerState.h"
#include "SGSpectatorPawn.h"
#include "Camera/CameraComponent.h"


ASGGameMode::ASGGameMode()
{
	bDelayedStart = true;
}

bool ASGGameMode::MustSpectate_Implementation(APlayerController* NewPlayerController) const
{
	const ASGGameState* DetailedGameState = GetGameState<ASGGameState>();

	return DetailedGameState->GetShooterMatchState() == EMatchState::InProgress || Super::MustSpectate_Implementation(
		NewPlayerController);
}

void ASGGameMode::StartMatch()
{
	Super::StartMatch();

	ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthSetMatchState(EMatchState::InProgress);
	DetailedGameState->OnKill.AddUniqueDynamic(this, &ASGGameMode::HandlePlayerKill);

	USGGameInstance* GameInstance = GetWorld()->GetGameInstance<USGGameInstance>();
	check(IsValid(GameInstance))
	GameInstance->LockSession();
}

void ASGGameMode::EndMatch()
{
	Super::EndMatch();

	ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthSetMatchState(EMatchState::Finished);
}

void ASGGameMode::ShuffleActors(TArray<AActor*>& Actors)
{
	for (int32 i = 0; i < Actors.Num() - 1; ++i)
	{
		const int32 SwapIndex = FMath::RandRange(0, Actors.Num() - 1);
		Actors.Swap(i, SwapIndex);
	}
}

void ASGGameMode::ResetPlayer(ASGPlayerState* Player)
{
	ASGPlayerController* Controller = Player->GetOwner<ASGPlayerController>();
	check(IsValid(Controller))

	const AActor* PlayerStart = FindPlayerStart(Controller);

	if (!IsValid(PlayerStart))
	{
		return;
	}

	ASGCharacter* Character = Player->GetCharacter();
	check(IsValid(Character))

	if (Player->IsDead())
	{
		ASGSpectatorPawn* SpectatorPawn = Controller->GetPawn<ASGSpectatorPawn>();
		check(IsValid(SpectatorPawn))

		SpectatorPawn->ClientHideWidgets();
		SpectatorPawn->Destroy();

		Controller->Possess(Character);
		Player->SetIsSpectator(false);
	}

	Controller->ClientResetControlRotation(PlayerStart->GetActorRotation());
	Character->AuthReset(PlayerStart);
}

void ASGGameMode::HandlePlayerKill(ASGPlayerState*, ASGPlayerState* Victim, bool)
{
	const ASGCharacter* VictimCharacter = Victim->GetPawn<ASGCharacter>();
	check(IsValid(VictimCharacter))

	const FVector Location = VictimCharacter->GetCamera()->GetComponentLocation();
	const FRotator Rotation = VictimCharacter->GetActorRotation();

	ASpectatorPawn* SpectatorPawn = GetWorld()->SpawnActor<ASpectatorPawn>(SpectatorClass, Location, Rotation);
	VictimCharacter->GetController()->Possess(SpectatorPawn);
}
