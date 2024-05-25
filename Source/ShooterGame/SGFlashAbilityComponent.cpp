#include "SGFlashAbilityComponent.h"

#include "SGCharacter.h"
#include "SGFlash.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

USGFlashAbilityComponent::USGFlashAbilityComponent()
{
	Cooldown = 7.f;
	MaxDeployDistance = 1500.f;
}

void USGFlashAbilityComponent::ServerUtilize_Implementation()
{
	if (!CanBeUtilized()) return;

	ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	const FVector Start = OwningCharacter->GetCamera()->GetComponentLocation();
	const FVector End = Start + UKismetMathLibrary::GetForwardVector(OwningCharacter->GetBaseAimRotation()) *
		MaxDeployDistance;

	FHitResult HitResult;
	const bool bHit = UKismetSystemLibrary::LineTraceSingle(this, Start, End,
	                                                        UEngineTypes::ConvertToTraceType(ECC_Visibility), false, {},
	                                                        EDrawDebugTrace::ForDuration, HitResult, true,
	                                                        FLinearColor::Yellow,
	                                                        FLinearColor::Blue, 2.f);
	if (!bHit) return;

	Super::ServerUtilize_Implementation();

	FActorSpawnParameters FlashSpawnParameters;
	FlashSpawnParameters.Owner = OwningCharacter;
	FlashSpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	check(IsValid(FlashClass))

	GetWorld()->SpawnActor<AActor>(FlashClass, FVector(HitResult.Location), UKismetMathLibrary::MakeRotFromZ(HitResult.Normal),
	                       FlashSpawnParameters);
}
