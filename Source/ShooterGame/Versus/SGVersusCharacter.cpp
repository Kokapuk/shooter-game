#include "SGVersusCharacter.h"

#include "SGVersusGameMode.h"
#include "SGVersusPlayerState.h"
#include "Net/UnrealNetwork.h"

void ASGVersusCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGVersusCharacter, Team);
}

void ASGVersusCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	const ASGVersusPlayerState* DetailedPlayerState = NewController->GetPlayerState<ASGVersusPlayerState>();
	check(IsValid(DetailedPlayerState))

	Team = DetailedPlayerState->GetTeam();
	OnRep_Team();
}

bool ASGVersusCharacter::ShouldTakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser) const
{
	const ASGVersusGameMode* GameMode = GetWorld()->GetAuthGameMode<ASGVersusGameMode>();
	check(IsValid(GameMode))

	const ASGVersusCharacter* Damager = Cast<ASGVersusCharacter>(DamageCauser);
	check(IsValid(Damager))

	if (!GameMode->IsFriendlyFireAllowed() && Damager->GetTeam() == GetTeam())
	{
		return false;
	}
	
	return Super::ShouldTakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

ETeam ASGVersusCharacter::GetTeam() const
{
	return Team;
}

void ASGVersusCharacter::OnRep_Team()
{
	check(IsValid(RedTeamMaterial))
	check(IsValid(BlueTeamMaterial))

	USkeletalMeshComponent* CharacterMesh = GetMesh();

	if (Team == ETeam::Red)
	{
		CharacterMesh->SetMaterial(0, RedTeamMaterial);
		ArmsMesh->SetMaterial(0, RedTeamMaterial);
	}
	else if (Team == ETeam::Blue)
	{
		CharacterMesh->SetMaterial(0, BlueTeamMaterial);
		ArmsMesh->SetMaterial(0, BlueTeamMaterial);
	}
}