#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGTracer.generated.h"

class UProjectileMovementComponent;

UCLASS()
class SHOOTERGAME_API ASGTracer : public AActor
{
	GENERATED_BODY()

public:
	ASGTracer();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetDistanceToTravel(const float NewDistanceToTravel) { DistanceToTravel = NewDistanceToTravel; }

protected:
	UPROPERTY(EditDefaultsOnly)
	UParticleSystemComponent* ParticleSystem;

	UPROPERTY(EditDefaultsOnly)
	UProjectileMovementComponent* ProjectileMovement;

private:
	float DistanceToTravel;
	FVector StartLocation;

	UFUNCTION()
	void HandleSystemFinished(UParticleSystemComponent* ParticleSystemComponent) { Destroy(); }
};
