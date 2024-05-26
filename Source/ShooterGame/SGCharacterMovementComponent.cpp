#include "SGCharacterMovementComponent.h"

#include "SGCharacter.h"
#include "SGDashAbilityComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

USGCharacterMovementComponent::USGCharacterMovementComponent(const FObjectInitializer& ObjectInitializer) : Super(
	ObjectInitializer)
{
	bCanWalkOffLedgesWhenCrouching = true;
	NavAgentProps.bCanCrouch = true;
	MaxWalkSpeed = 540.f;
	MaxWalkSpeedCrouched = 240.f;
	AirControl = 1.f;
	CrouchedHalfHeight = 60.f;
	PerchRadiusThreshold = 10.f;

	bWantsToDash = false;
	LastDashWorldTime = 0.f;
	SavedMaxAcceleration = MaxAcceleration;
}

void USGCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToDash = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

FNetworkPredictionData_Client* USGCharacterMovementComponent::GetPredictionData_Client() const
{
	check(IsValid(PawnOwner));

	if (!ClientPredictionData)
	{
		USGCharacterMovementComponent* MutableThis = const_cast<USGCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_SGCharacter(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

bool USGCharacterMovementComponent::CanDash() const
{
	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	check(IsValid(OwningCharacter))

	const USGDashAbilityComponent* DashAbilityComponent = OwningCharacter->GetAbilityComponent<
		USGDashAbilityComponent>();
	check(IsValid(DashAbilityComponent))

	const float WorldTime = GetWorld()->GetTimeSeconds();

	if (LastDashWorldTime > 0.f && LastDashWorldTime + DashAbilityComponent->GetCooldown() > WorldTime)
	{
		return false;
	}

	FVector DashDirection = Velocity.GetSafeNormal();
	DashDirection.Z = 0.f;

	if (DashDirection == FVector::ZeroVector) return false;

	return true;
}

void USGCharacterMovementComponent::AuthReset()
{
	if (!GetOwner()->HasAuthority()) return;

	Reset();
	ClientReset();
}

void USGCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
                                                      const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	if (!CharacterOwner) return;

	if (bWantsToDash) PerformDash();
}

void USGCharacterMovementComponent::PerformDash()
{
	bWantsToDash = false;

	if (!CanDash()) return;

	const ASGCharacter* OwningCharacter = Cast<ASGCharacter>(GetOwner());
	const USGDashAbilityComponent* DashAbilityComponent = OwningCharacter->GetAbilityComponent<
		USGDashAbilityComponent>();

	MaxAcceleration = 0;
	LastDashWorldTime = GetWorld()->GetTimeSeconds();

	const TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
	ConstantForce->InstanceName = "DashConstantForce";
	ConstantForce->AccumulateMode = ERootMotionAccumulateMode::Additive;
	ConstantForce->Priority = 5;
	ConstantForce->Force = DashAbilityComponent->GetDistance() / DashAbilityComponent->GetDuration() * Velocity.GetSafeNormal();
	ConstantForce->Duration = DashAbilityComponent->GetDuration();
	ConstantForce->StrengthOverTime = DashAbilityComponent->GetCurve();
	ConstantForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::ClampVelocity;
	ConstantForce->FinishVelocityParams.SetVelocity = FVector::ZeroVector;
	ConstantForce->FinishVelocityParams.ClampVelocity = 1000.f;
	ConstantForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);

	ApplyRootMotionSource(ConstantForce);

	GetWorld()->GetTimerManager().SetTimer(DashTimerHandle, this, &USGCharacterMovementComponent::FinishDash,
	                                       DashAbilityComponent->GetDuration());
}

void USGCharacterMovementComponent::Reset()
{
	LastDashWorldTime = 0.f;
	RemoveRootMotionSource("DashConstantForce");
	GetWorld()->GetTimerManager().ClearTimer(DashTimerHandle);
	MaxAcceleration = SavedMaxAcceleration;
}

void USGCharacterMovementComponent::ClientReset_Implementation()
{
	Reset();
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

	const USGCharacterMovementComponent* CharacterMovement = Cast<USGCharacterMovementComponent>(
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
