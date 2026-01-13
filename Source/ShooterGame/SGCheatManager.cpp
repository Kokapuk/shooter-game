#include "SGCheatManager.h"

#include "SGCharacter.h"
#include "Kismet/GameplayStatics.h"


void USGCheatManager::ServerApplyDamage_Implementation(const float DamageAmount)
{
	ASGCharacter* Character = Cast<ASGCharacter>(GetPlayerController()->GetCharacter());
	if (!IsValid(Character)) return;

	UGameplayStatics::ApplyDamage(Character, DamageAmount, GetPlayerController(), Character,
	                              UDamageType::StaticClass());
}


void USGCheatManager::ServerSetTimeScale_Implementation(const float NewTimeScale)
{
	MultiSetTimeScale(NewTimeScale);
}

void USGCheatManager::MultiSetTimeScale_Implementation(const float NewTimeScale)
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), NewTimeScale);
}
