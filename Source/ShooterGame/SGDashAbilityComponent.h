#pragma once

#include "CoreMinimal.h"
#include "SGAbilityComponent.h"
#include "SGDashAbilityComponent.generated.h"

class USoundCue;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="Dash Ability Component", Blueprintable)
class SHOOTERGAME_API USGDashAbilityComponent : public USGAbilityComponent
{
	GENERATED_BODY()

public:
	USGDashAbilityComponent();

	virtual bool CanBeUtilized() const override;

	UFUNCTION(BlueprintPure)
	float GetDistance() const { return Distance; }

	UFUNCTION(BlueprintPure)
	UCurveFloat* GetCurve() const;

	UFUNCTION(BlueprintPure)
	float GetDuration() const;

protected:
	virtual void CosmeticUtilize() override;
	virtual void ServerUtilize_Implementation() override;

	void PlaySoundCue() const;

	UFUNCTION(NetMulticast, Unreliable)
	void MultiPlaySoundCue();

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.f))
	float Distance;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* Curve;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* SoundCue;
};
