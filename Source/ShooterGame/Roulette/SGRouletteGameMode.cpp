#include "SGRouletteGameMode.h"

#include "EngineUtils.h"
#include "SGRouletteWeaponComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterGame/SGCharacter.h"
#include "ShooterGame/SGPlayerStart.h"

ASGRouletteGameMode::ASGRouletteGameMode()
{
	RespawnTime = 5.f;
}

AActor* ASGRouletteGameMode::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	AActor* FurthestPlayerState = nullptr;
	float MaxDistanceToClosestPlayer = -2.f;

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, ASGPlayerStart::StaticClass(), PlayerStarts);
	ShuffleActors(PlayerStarts);

	for (AActor* PlayerStart : PlayerStarts)
	{
		float DistanceToClosestPlayer = -1.f;

		for (TActorIterator<ASGCharacter> Character(GetWorld()); Character; ++Character)
		{
			if (Character->IsDead())
			{
				continue;
			}

			const float Distance = PlayerStart->GetDistanceTo(*Character);

			if (DistanceToClosestPlayer > -1.f && Distance >= DistanceToClosestPlayer)
			{
				continue;
			}

			DistanceToClosestPlayer = Distance;
		}

		if (DistanceToClosestPlayer > MaxDistanceToClosestPlayer)
		{
			FurthestPlayerState = PlayerStart;
			MaxDistanceToClosestPlayer = DistanceToClosestPlayer;
		}
	}

	return FurthestPlayerState;
}

void ASGRouletteGameMode::StartMatch()
{
	Super::StartMatch();

	for (TActorIterator<ASGCharacter> Character(GetWorld()); Character; ++Character)
	{
		USGWeaponComponent* Weapon = Character->GetWeaponComponent();
		check(IsValid(Weapon))

		Weapon->OnFire.AddUniqueDynamic(this, &ASGRouletteGameMode::GiveBulletToRandomCharacter);
	}

	GiveBulletToRandomCharacter();
}

void ASGRouletteGameMode::HandlePlayerKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot)
{
	Super::HandlePlayerKill(Killer, Victim, bIsHeadshot);

	FTimerDelegate RespawnDelegate;
	RespawnDelegate.BindUFunction(this, FName("ResetPlayer"), Victim);

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, RespawnDelegate, RespawnTime, false);
}

void ASGRouletteGameMode::GiveBulletToRandomCharacter()
{
	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(this, ASGCharacter::StaticClass(), Characters);
	
	if (Characters.Num() == 0) return;

	ShuffleActors(Characters);
	const ASGCharacter* Character = Cast<ASGCharacter>(Characters[0]);

	USGRouletteWeaponComponent* Weapon = Character->GetWeaponComponent<USGRouletteWeaponComponent>();
	check(IsValid(Weapon))

	Weapon->SetRounds(1);
}
