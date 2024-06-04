#include "SGVersusPlayerStart.h"

#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"

ASGVersusPlayerStart::ASGVersusPlayerStart(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	Capsule->SetCapsuleHalfHeight(96.f);
	Capsule->SetCapsuleRadius(55.f);
}

void ASGVersusPlayerStart::AuthOccupy(APlayerController* Player)
{
	if (!HasAuthority()) return;
	
	OccupiedBy = Player;
}
