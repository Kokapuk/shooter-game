#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SGCharacterMovementComponent.generated.h"

struct FTimeline;

UCLASS()
class SHOOTERGAME_API USGCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_UCLASS_BODY()
	class FSavedMove_SGCharacter : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
		                        FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* Character) override;

		uint8 bSavedWantsToDash : 1;
	};

	class FNetworkPredictionData_Client_SGCharacter : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_SGCharacter(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override { return FSavedMovePtr(new FSavedMove_SGCharacter()); }
	};

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	UFUNCTION(BlueprintCallable)
	void Dash() { bWantsToDash = true; }

	UFUNCTION(BlueprintPure)
	bool CanDash() const;

	UFUNCTION(BlueprintCallable, DisplayName="Reset", BlueprintAuthorityOnly)
	void AuthReset() { LastDashWorldTime = 0.f; }

	uint8 bWantsToDash : 1;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	void PerformDash();

	UFUNCTION()
	void FinishDash() { MaxAcceleration = SavedMaxAcceleration; }

private:
	float LastDashWorldTime;
	float SavedMaxAcceleration;
};
