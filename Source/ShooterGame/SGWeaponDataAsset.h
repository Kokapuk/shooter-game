#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SGWeaponDataAsset.generated.h"

class ASGTracer;

UCLASS()
class SHOOTERGAME_API USGWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	USGWeaponDataAsset();

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(ClampMin=0.f))
	float TimeBetweenShots;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(ClampMin=0.f))
	float MaxShootingError;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(ClampMin=0.f))
	float ReloadTime;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, meta=(ClampMin=0))
	int32 MagazineCapacity;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float HeadShotDamage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	float BodyShotDamage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USkeletalMesh* Mesh;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* WeaponFireMontage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* FirstPersonFireMontage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* WeaponReloadMontage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* ThirdPersonReloadMontage;

  UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UAnimMontage* FirstPersonReloadMontage;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<ASGTracer> TracerClass;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USoundBase* SurfaceImpactCue;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UParticleSystem* SurfaceImpactParticles;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	USoundBase* BodyImpactCue;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UParticleSystem* BodyImpactParticles;
};
