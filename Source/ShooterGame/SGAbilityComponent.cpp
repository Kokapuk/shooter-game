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

	DOREPLIFETIME_CONDITION(USGAbilityComponent, RemainingCooldownTime, COND_OwnerOnly)
}

void USGAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!GetOwner()->HasAuthority()) return;

	RemainingCooldownTime = FMath::Clamp(RemainingCooldownTime - DeltaTime, 0.f, CooldownTime);
}

void USGAbilityComponent::ServerUtilize_Implementation()
{
	RemainingCooldownTime = CooldownTime;
}
