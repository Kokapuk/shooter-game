#include "SGCharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

USGCharacterMovementComponent::USGCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer)
{
	DashStrength = 1400.f;
	bWantsToDash = false;
}

void USGCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToDash = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* USGCharacterMovementComponent::GetPredictionData_Client() const
{
	check(IsValid(PawnOwner));
	// check(PawnOwner->GetLocalRole() < ROLE_Authority);

	if (!ClientPredictionData)
	{
		USGCharacterMovementComponent* MutableThis = const_cast<USGCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_SGCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void USGCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
                                                      const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner)
	{
		return;
	}

	if (bWantsToDash)
	{
		bWantsToDash = false;

		GroundFriction = 0.f;

		FVector DodgeVelocity = UKismetMathLibrary::Normal(Velocity) * DashStrength;
		DodgeVelocity.Z = 0.0f;

		CharacterOwner->LaunchCharacter(DodgeVelocity, false, false);

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, this, &USGCharacterMovementComponent::StopDash, .45f);
	}
}

void USGCharacterMovementComponent::Dash()
{
	bWantsToDash = true;
}

void USGCharacterMovementComponent::StopDash()
{
	GroundFriction = 8.f;
	Velocity = UKismetMathLibrary::Normal(Velocity) * MaxWalkSpeed;
}

bool USGCharacterMovementComponent::FSavedMove_SGCharacter::CanCombineWith(
	const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	if (bSavedWantsToDash != ((FSavedMove_SGCharacter*)&NewMove)->bSavedWantsToDash)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void USGCharacterMovementComponent::FSavedMove_SGCharacter::Clear()
{
	Super::Clear();

	bSavedWantsToDash = false;
}

uint8 USGCharacterMovementComponent::FSavedMove_SGCharacter::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedWantsToDash)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void USGCharacterMovementComponent::FSavedMove_SGCharacter::SetMoveFor(ACharacter* Character, float InDeltaTime,
                                                                       FVector const& NewAccel,
                                                                       FNetworkPredictionData_Client_Character&
                                                                       ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	USGCharacterMovementComponent* CharacterMovement = Cast<USGCharacterMovementComponent>(
		Character->GetCharacterMovement());

	if (CharacterMovement)
	{
		bSavedWantsToDash = CharacterMovement->bWantsToDash;
	}
}

void USGCharacterMovementComponent::FSavedMove_SGCharacter::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	USGCharacterMovementComponent* CharacterMovement = Cast<USGCharacterMovementComponent>(
		Character->GetCharacterMovement());

	if (CharacterMovement)
	{
		CharacterMovement->bWantsToDash = bSavedWantsToDash;
	}
}

USGCharacterMovementComponent::FNetworkPredictionData_Client_SGCharacter::FNetworkPredictionData_Client_SGCharacter(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}
