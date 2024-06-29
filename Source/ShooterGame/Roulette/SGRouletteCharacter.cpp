#include "SGRouletteCharacter.h"

#include "SGRouletteWeaponComponent.h"

ASGRouletteCharacter::ASGRouletteCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<USGRouletteWeaponComponent>("Weapon"))
{
}
