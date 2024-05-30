#include "SGAbilityComponent.h"

#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"

USGAbilityComponent::USGAbilityComponent()
{
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
}

void USGAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USGAbilityComponent, CoolsDownOn, COND_SkipOwner);
}

bool USGAbilityComponent::CanBeUtilized() const
{
	return CoolsDownOn <= GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
}

float USGAbilityComponent::GetRemainingCooldown() const
{
	return FMath::Clamp(CoolsDownOn - GetWorld()->GetGameState()->GetServerWorldTimeSeconds(), 0.f, Cooldown);
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

	CoolsDownOn = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() + Cooldown;
}

void USGAbilityComponent::ServerUtilize_Implementation()
{
	if (!CanBeUtilized()) return;

	CoolsDownOn = GetWorld()->GetGameState()->GetServerWorldTimeSeconds() + Cooldown;
}
