#include "SGPlayerStart.h"

#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"

ASGPlayerStart::ASGPlayerStart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCapsuleHalfHeight(96.f);
	Capsule->SetCapsuleRadius(55.f);
}

void ASGPlayerStart::AuthOccupy(APlayerController* Player)
{
	if (!HasAuthority()) return;
	
	OccupiedBy = Player;
}
