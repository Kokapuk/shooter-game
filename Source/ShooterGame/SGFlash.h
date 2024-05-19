#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "GameFramework/Actor.h"
#include "SGFlash.generated.h"

class USoundCue;
struct FTimeline;
class UPointLightComponent;

UCLASS()
class SHOOTERGAME_API ASGFlash : public AActor
{
	GENERATED_BODY()

public:
	ASGFlash();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UPROPERTY(EditDefaultsOnly)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditDefaultsOnly)
	UPointLightComponent* Light;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.f))
	float MaxScale;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.f))
	float MaxLightIntensity;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.f))
	float MaxLocationOffset;
	
	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* ChargeCurve;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* ChargingCue;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* ExplosionCue;

	UPROPERTY(EditDefaultsOnly)
	UCurveFloat* BlindnessCurve;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* BlindedCue;

	UFUNCTION()
	void HandleChargeProgress();
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="Explode")
	void AuthExplode();

	UFUNCTION(NetMulticast, Reliable)
	void MultiExplode(const bool bBlinded);

private:
	FTimeline ChargeTimeline;
	FVector StartLocation;
};
