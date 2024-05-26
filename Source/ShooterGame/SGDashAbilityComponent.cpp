#include "SGDashAbilityComponent.h"

#include "SGCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

USGDashAbilityComponent::USGDashAbilityComponent()
{
	Cooldown = 7.f;
	Distance = 1000.f;
	Curve = nullptr;
	SoundCue = nullptr;
}

bool USGDashAbilityComponent::CanBeUtilized() const
{
	const ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	const USGCharacterMovementComponent* CharacterMovement = Cast<USGCharacterMovementComponent>(
		OwningCharacter->GetCharacterMovement());
	check(IsValid(CharacterMovement))

	return CharacterMovement->CanDash() && Super::CanBeUtilized();
}

UCurveFloat* USGDashAbilityComponent::GetCurve() const
{
	check(IsValid(Curve))

	return Curve;
}

float USGDashAbilityComponent::GetDuration() const
{
	check(IsValid(Curve))

	float _, MaxTime;

	Curve->GetTimeRange(_, MaxTime);

	return MaxTime;
}

void USGDashAbilityComponent::CosmeticUtilize()
{
	Super::CosmeticUtilize();

	const ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	USGCharacterMovementComponent* CharacterMovement = Cast<USGCharacterMovementComponent>(
		OwningCharacter->GetCharacterMovement());
	check(IsValid(CharacterMovement));

	CharacterMovement->Dash();
	PlaySoundCue();
}

void USGDashAbilityComponent::ServerUtilize_Implementation()
{
	if (!CanBeUtilized()) return;

	Super::ServerUtilize_Implementation();

	MultiPlaySoundCue();
}

void USGDashAbilityComponent::PlaySoundCue() const
{
	const ACharacter* OwningCharacter = Cast<ACharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	USkeletalMeshComponent* CharacterMesh = OwningCharacter->GetMesh();
	check(IsValid(CharacterMesh))

	check(IsValid(SoundCue))
	UGameplayStatics::SpawnSoundAttached(SoundCue, CharacterMesh);
}

void USGDashAbilityComponent::MultiPlaySoundCue_Implementation()
{
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	check(IsValid(OwningPawn))

	if (OwningPawn->IsLocallyControlled()) return;

	PlaySoundCue();
}
