#include "SGGameMode.h"

#include "SGCharacter.h"
#include "SGGameState.h"
#include "SGPlayerController.h"
#include "SGPlayerStart.h"
#include "SGPlayerState.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"

ASGGameMode::ASGGameMode()
{
	bDelayedStart = true;
	bIsFriendlyFireAllowed = false;
}

void ASGGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ASGGameState* ShooterGameState = GetGameState<ASGGameState>();
	if (!IsValid(ShooterGameState)) return;

	APlayerState* Player = NewPlayer->GetPlayerState<APlayerState>();
	if (!IsValid(Player)) return;

	ShooterGameState->AuthRegisterPlayerInTeam(Player, ETeam::None);
}

UClass* ASGGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const ASGPlayerState* PlayerState = InController->GetPlayerState<ASGPlayerState>();
	
	if (!IsValid(PlayerState) || PlayerState->GetTeam() == ETeam::None)
	{
		return SpectatorClass;
	}

	return DefaultPawnClass;
}

AActor* ASGGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	const ASGPlayerState* PlayerState = Player->GetPlayerState<ASGPlayerState>();
	if (!IsValid(PlayerState)) return nullptr;

	APlayerController* PlayerController = Cast<APlayerController>(Player);
	if (!IsValid(PlayerController)) return nullptr;

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
	if (IsValid(DetailedGameState)) DetailedGameState->AuthSetMatchState(EMatchState::InProgress);
}

void ASGGameMode::ResetPlayers()
{
	const ASGGameState* DetailedGameState = GetGameState<ASGGameState>();
	if (!IsValid(DetailedGameState)) return;
	
	TArray<APlayerState*> Players = DetailedGameState->GetAllPlayers();

	for (int32 i = 0; i < Players.Num(); ++i)
	{
		ASGPlayerState* PlayerState = Cast<ASGPlayerState>(Players[i]);
		if (!IsValid(PlayerState)) continue;

		ASGPlayerController* Controller = Cast<ASGPlayerController>(PlayerState->GetOwner());
		if (!IsValid(Controller)) continue;

		const AActor* PlayerStart = FindPlayerStart(Controller);
		if (!IsValid(PlayerStart)) continue;
		
		ASGCharacter* Character = Cast<ASGCharacter>(Controller->GetPawn());
		if (!IsValid(Character)) continue;

		PlayerState->AuthReset();
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
