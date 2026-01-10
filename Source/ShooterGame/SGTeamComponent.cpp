#include "SGTeamComponent.h"

#include "SGCharacter.h"
#include "Net/UnrealNetwork.h"

USGTeamComponent::USGTeamComponent()
{
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
}

void USGTeamComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USGTeamComponent, Team);
}

void USGTeamComponent::AuthSetTeam(const ETeam NewTeam)
{
	if (!GetOwner()->HasAuthority()) return;

	Team = NewTeam;
	OnRep_Team();
}

void USGTeamComponent::OnRep_Team()
{
	check(IsValid(RedTeamMaterial))
	check(IsValid(BlueTeamMaterial))

	const ASGCharacter* Character = GetOwner<ASGCharacter>();
	check(IsValid(Character))

	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	USkeletalMeshComponent* ArmsMesh = Character->GetArmsMesh();
	check(IsValid(CharacterMesh))
	check(IsValid(ArmsMesh))

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
