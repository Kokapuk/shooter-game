#include "SGAbilityComponent.h"

USGAbilityComponent::USGAbilityComponent()
{
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
}

void USGAbilityComponent::Utilize()
{
	if (!CanBeUtilized()) return;
	
	ServerUtilize();
	CosmeticUtilize();
}

void USGAbilityComponent::AuthReset()
{
	if (!GetOwner()->HasAuthority()) return;

	CoolsDownOn = 0.f;
	ClientReset();
}

void USGAbilityComponent::ClientReset_Implementation()
{
	CoolsDownOn = 0.f;
}

void USGAbilityComponent::CosmeticUtilize()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	check(IsValid(OwningPawn))

	if (!OwningPawn->IsLocallyControlled()) return;

	CoolsDownOn = GetWorld()->GetTimeSeconds() + Cooldown;
}

void USGAbilityComponent::ServerUtilize_Implementation()
{
	if (!CanBeUtilized()) return;

	CoolsDownOn = GetWorld()->GetTimeSeconds() + Cooldown;
}
