#include "SGRouletteWeaponComponent.h"

void USGRouletteWeaponComponent::SetRounds(const int32 NewRounds)
{
	Rounds = NewRounds;
}

void USGRouletteWeaponComponent::AuthResetRounds()
{
}

bool USGRouletteWeaponComponent::CanReload() const
{
	return false;
}
