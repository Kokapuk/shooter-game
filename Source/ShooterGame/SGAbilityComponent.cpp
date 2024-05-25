#include "SGAbilityComponent.h"

#include "Net/UnrealNetwork.h"

USGAbilityComponent::USGAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
}

void USGAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USGAbilityComponent, RemainingCooldown, COND_OwnerOnly)
}

void USGAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

	RemainingCooldown = FMath::Clamp(RemainingCooldown - DeltaTime, 0.f, Cooldown);
}

void USGAbilityComponent::Utilize()
{
	CosmeticUtilize();
	ServerUtilize();
}

void USGAbilityComponent::ServerUtilize_Implementation()
{
	if (!CanBeUtilized()) return;
	
	RemainingCooldown = Cooldown;
}
