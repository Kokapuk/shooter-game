#include "SGVersusGameMode.h"

#include "SGVersusGameState.h"
#include "SGVersusPlayerStart.h"
#include "SGVersusPlayerState.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterGame/SGAbilityDataAsset.h"
#include "ShooterGame/SGCharacter.h"

ASGVersusGameMode::ASGVersusGameMode()
{
	bIsFriendlyFireAllowed = false;
	RoundsToWin = 10;
	PostRoundTime = 3.f;
}

void ASGVersusGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	ASGVersusPlayerState* Player = NewPlayer->GetPlayerState<ASGVersusPlayerState>();
	check(IsValid(Player))

	Player->ServerRegisterPlayerInTeam(ETeam::None);
}

UClass* ASGVersusGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	const ASGVersusPlayerState* PlayerState = InController->GetPlayerState<ASGVersusPlayerState>();
	check(IsValid(PlayerState));

	if (PlayerState->GetTeam() == ETeam::None)
	{
		return SpectatorClass;
	}

	return DefaultPawnClass;
}

AActor* ASGVersusGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	const ASGVersusPlayerState* PlayerState = Player->GetPlayerState<ASGVersusPlayerState>();
	check(IsValid(PlayerState));

	APlayerController* PlayerController = Cast<APlayerController>(Player);

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, ASGVersusPlayerStart::StaticClass(), PlayerStarts);
	ShuffleActors(PlayerStarts);

	for (int32 i = 0; i < PlayerStarts.Num(); ++i)
	{
		ASGVersusPlayerStart* PlayerStart = Cast<ASGVersusPlayerStart>(PlayerStarts[i]);
		if (PlayerStart->GetOccupiedBy() == Player) return PlayerStart;
		if (PlayerStart->IsOccupied() || PlayerStart->GetTeam() != PlayerState->GetTeam()) continue;

		if (PlayerStart->GetTeam() != ETeam::None) PlayerStart->AuthOccupy(PlayerController);
		return PlayerStart;
	}

	return nullptr;
}

void ASGVersusGameMode::StartMatch()
{
	Super::StartMatch();

	ASGVersusGameState* DetailedGameState = GetGameState<ASGVersusGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthSetRoundState(ERoundState::InProgress);
}

bool ASGVersusGameMode::ShouldTakeDamage(const ASGCharacter* Damager, const ASGCharacter* Target) const
{
	const USGTeamComponent* DamagerTeamComponent = Damager->GetComponentByClass<USGTeamComponent>();
	const USGTeamComponent* TargetTeamComponent = Target->GetComponentByClass<USGTeamComponent>();

	if (!IsFriendlyFireAllowed() && DamagerTeamComponent->GetTeam() == TargetTeamComponent->GetTeam())
	{
		return false;
	}

	return Super::ShouldTakeDamage(Damager, Target);
}

void ASGVersusGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ASGVersusPlayerState* Player = Exiting->GetPlayerState<ASGVersusPlayerState>();
	check(IsValid(Player))

	Player->ServerUnregisterPlayerFromTeam(Player->GetTeam());
}

void ASGVersusGameMode::FinishRound()
{
	ASGVersusGameState* DetailedGameState = GetGameState<ASGVersusGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthSetRoundState(ERoundState::Finished);

	if (DetailedGameState->GetTeamScore(ETeam::Red) == RoundsToWin || DetailedGameState->GetTeamScore(ETeam::Blue) ==
		RoundsToWin)
	{
		return EndMatch();
	}

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, this, &ASGVersusGameMode::StartNewRound, PostRoundTime, false);
}

void ASGVersusGameMode::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	Super::FinishRestartPlayer(NewPlayer, StartRotation);

	ASGVersusPlayerState* PlayerState = NewPlayer->GetPlayerState<ASGVersusPlayerState>();

	if (PlayerState->GetTeam() == ETeam::None) return;

	ASGCharacter* Character = NewPlayer->GetPawn<ASGCharacter>();
	check(IsValid(Character))
	check(IsValid(PlayerState))

	USGWeaponComponent* WeaponComponent = Character->GetWeaponComponent();
	USGAbilityDataAsset* Ability = PlayerState->GetAbility();
	check(IsValid(WeaponComponent))
	check(IsValid(Ability))

	USGTeamComponent* TeamComponent = NewObject<USGTeamComponent>(Character, TeamComponentClass);
	TeamComponent->RegisterComponent();

	WeaponComponent->ServerEquip(DefaultWeapon);
	Character->ServerSetAbility(Ability);
	TeamComponent->AuthSetTeam(PlayerState->GetTeam());
}

void ASGVersusGameMode::ResetPlayers()
{
	const ASGVersusGameState* DetailedGameState = GetGameState<ASGVersusGameState>();
	check(IsValid(DetailedGameState))

	for (ASGVersusPlayerState* Player : DetailedGameState->GetPlayers())
	{
		ResetPlayer(Player);
	}
}

void ASGVersusGameMode::StartNewRound()
{
	ResetPlayers();

	ASGVersusGameState* DetailedGameState = GetGameState<ASGVersusGameState>();
	check(IsValid(DetailedGameState))

	DetailedGameState->AuthSetRoundState(ERoundState::InProgress);
}
