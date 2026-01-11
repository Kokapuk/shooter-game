#include "SGCheatManager.h"

#include "Kismet/GameplayStatics.h"

void USGCheatManager::ApplyDamage(const float DamageAmount)
{
	ASGCharacter* Character = Cast<ASGCharacter>(GetPlayerController()->GetCharacter());
	if (!IsValid(Character)) return;

	AuthApplyDamage(DamageAmount, Character);
}

void USGCheatManager::AuthApplyDamage_Implementation(const float DamageAmount, ASGCharacter* Character)
{
	UGameplayStatics::ApplyDamage(Character, DamageAmount, GetPlayerController(), Character,
	                              UDamageType::StaticClass());
}
