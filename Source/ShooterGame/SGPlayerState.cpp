#include "SGPlayerState.h"

#include "Net/UnrealNetwork.h"

void ASGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGPlayerState, Team);
}

void ASGPlayerState::AuthAssignTeam(const ETeam NewTeam)
{
	if (!HasAuthority()) return;

	Team = NewTeam;
}

UMaterialInterface* ASGPlayerState::GetMaterialByTeam() const
{
	switch (Team)
	{
	case ETeam::Red:
		return RedTeamMaterial;
	case ETeam::Blue:
		return BlueTeamMaterial;
	default:
		return NoTeamMaterial;
	}
}

// void ASGPlayerState::OnRep_Team()
// {
// 	const ASGCharacter* ControlledCharacter = GetControlledCharacter();
// 	if (!IsValid(ControlledCharacter)) return;
//
// 	UMaterialInterface* CurrentTeamMaterial = GetMaterialByTeam();
// 	if (!IsValid(CurrentTeamMaterial)) return;
//
// 	if (!ControlledCharacter->IsLocallyControlled())
// 	{
// 		USkeletalMeshComponent* ControlledCharacterMesh = ControlledCharacter->GetMesh();
// 		if (!IsValid(ControlledCharacterMesh)) return;
//
// 		ControlledCharacterMesh->SetMaterial(0, CurrentTeamMaterial);
// 	}
// 	else
// 	{
// 		USkeletalMeshComponent* ControlledCharacterMesh = ControlledCharacter->GetArmsMesh();
// 		if (!IsValid(ControlledCharacterMesh)) return;
//
// 		ControlledCharacterMesh->SetMaterial(0, CurrentTeamMaterial);
// 	}
// }
