#include "SGGameMode.h"

#include "SGCharacter.h"
#include "SGGameState.h"
#include "SGPlayerController.h"
#include "SGPlayerStart.h"
#include "SGPlayerState.h"
#include "SGSpectatorPawn.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"

ASGGameMode::ASGGameMode()
{
	bDelayedStart = true;
	bIsFriendlyFireAllowed = false;
	RoundsToWin = 1;
}

void ASGGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	check(IsValid(DetailedGameState))

	APlayerState* Player = NewPlayer->GetPlayerState<APlayerState>();

	DetailedGameState->AuthRegisterPlayerInTeam(Player, ETeam::None);
}

UClass* ASGGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const ASGPlayerState* PlayerState = InController->GetPlayerState<ASGPlayerState>();
	check(IsValid(PlayerState));

	if (PlayerState->GetTeam() == ETeam::None)
	{
		return SpectatorClass;
	}
	
	return DefaultPawnClass;
}

AActor* ASGGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	const ASGPlayerState* PlayerState = Player->GetPlayerState<ASGPlayerState>();
	check(IsValid(PlayerState));

	APlayerController* PlayerController = Cast<APlayerController>(Player);

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, ASGPlayerStart::StaticClass(), PlayerStarts);
	ShufflePlayerStarts(PlayerStarts);

	for (int32 i = 0; i < PlayerStarts.Num(); ++i)
	{
		ASGPlayerStart* PlayerStart = Cast<ASGPlayerStart>(PlayerStarts[i]);
		if (PlayerStart->GetOccupiedBy() == Player) return PlayerStart;
		if (PlayerStart->IsOccupied() || PlayerStart->GetTeam() != PlayerState->GetTeam()) continue;

		if (PlayerStart->GetTeam() != ETeam::None) PlayerStart->AuthOccupy(PlayerController);
		return PlayerStart;
	}

	return nullptr;
}

void ASGGameMode::StartMatch()
{
	Super::StartMatch();

	ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthSetMatchState(EMatchState::InProgress);
	DetailedGameState->AuthSetRoundState(ERoundState::InProgress);
}

void ASGGameMode::EndMatch()
{
	Super::EndMatch();

	ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthSetMatchState(EMatchState::Finished);
}

void ASGGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ASGPlayerState* Player = Exiting->GetPlayerState<ASGPlayerState>();
	check(IsValid(Player))

	ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthUnregisterPlayerFromTeam(Player, Player->GetTeam());
}

void ASGGameMode::FinishRound()
{
	ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthSetRoundState(ERoundState::Finished);

	if (DetailedGameState->GetTeamScore(ETeam::Red) == RoundsToWin || DetailedGameState->GetTeamScore(ETeam::Blue) ==
		RoundsToWin)
	{
		return EndMatch();
	}

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, [this, DetailedGameState]()
	{
		ResetPlayers();
		DetailedGameState->AuthSetRoundState(ERoundState::InProgress);
	}, 3.f, false);
}

void ASGGameMode::ResetPlayers()
{
	const ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	check(IsValid(DetailedGameState))


	TArray<APlayerState*> Players = DetailedGameState->GetPlayers();

	for (int32 i = 0; i < Players.Num(); ++i)
	{
		ASGPlayerState* PlayerState = Cast<ASGPlayerState>(Players[i]);
		check(IsValid(PlayerState))

		ASGPlayerController* Controller = Cast<ASGPlayerController>(PlayerState->GetOwner());
		check(IsValid(Controller))

		const AActor* PlayerStart = FindPlayerStart(Controller);
		if (!IsValid(PlayerStart)) continue;

		ASGCharacter* Character = PlayerState->GetCharacter();
		check(IsValid(Character))

		if (PlayerState->IsDead())
		{
			ASGSpectatorPawn* SpectatorPawn = Controller->GetPawn<ASGSpectatorPawn>();
			check(IsValid(SpectatorPawn))

			SpectatorPawn->ClientHideWidgets();
			SpectatorPawn->Destroy();
			
			Controller->Possess(Character);
			PlayerState->SetIsSpectator(false);
		}

		Controller->ClientResetControlRotation(PlayerStart->GetActorRotation());
		Character->AuthReset(PlayerStart);
	}
}

void ASGGameMode::ShufflePlayerStarts(TArray<AActor*>& PlayerStarts)
{
	const int32 NumShuffles = PlayerStarts.Num() - 1;

	for (int32 i = 0; i < NumShuffles; ++i)
	{
		const int32 SwapIndex = FMath::RandRange(i, NumShuffles);
		PlayerStarts.Swap(i, SwapIndex);
	}
}
