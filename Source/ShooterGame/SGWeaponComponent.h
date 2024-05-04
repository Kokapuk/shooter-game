#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGWeaponComponent.generated.h"


class ASGCharacter;
class USGWeaponDataAsset;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SHOOTERGAME_API USGWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USGWeaponComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Equip")
	void ServerEquip(USGWeaponDataAsset* Weapon);

	UFUNCTION(BlueprintPure)
	USGWeaponDataAsset* GetEquipped() const { return Equipped; }

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="StartFire")
	void ServerStartFire();

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="StopFire")
	void ServerStopFire();

	UFUNCTION(BlueprintCallable, DisplayName="Fire", BlueprintAuthorityOnly)
	void AuthFire();

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Reload")
	void ServerReload();

	UFUNCTION(BlueprintPure)
	FVector GetFireDirection() const;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_Equipped)
	USGWeaponDataAsset* Equipped;

	bool bIsAutomaticallyFiring;
	float TimeToFire;
	float ShootingError;

	UFUNCTION(NetMulticast, Unreliable)
	void MultiFire(const FHitResult& HitResult);

	UFUNCTION(NetMulticast, Reliable)
	void MultiReload();

	void PlayFireAnimations() const;
	void SpawnTracer(const FHitResult& HitResult) const;
	void SpawnImpactParticles(const FHitResult& HitResult) const;

private:
	UFUNCTION()
	void OnRep_Equipped() const;
};
