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

	UFUNCTION(BlueprintCallable)
	void Equip(USGWeaponDataAsset* Weapon) { ServerEquip(Weapon); }

	UFUNCTION(BlueprintPure)
	USGWeaponDataAsset* GetEquipped() const { return Equipped; }

	UFUNCTION(BlueprintCallable)
	void Fire() { ServerFire(); }

	UFUNCTION(BlueprintCallable)
	void Reload() { ServerReload(); }

protected:
	UPROPERTY(ReplicatedUsing=OnRep_Equipped)
	USGWeaponDataAsset* Equipped;

	UFUNCTION(Server, Unreliable)
	void ServerEquip(USGWeaponDataAsset* Weapon);

	UFUNCTION(Server, Unreliable)
	void ServerFire();

	UFUNCTION(NetMulticast, Unreliable)
	void MultiFire(const FHitResult& HitResult);

	UFUNCTION(Server, Unreliable)
	void ServerReload();

	UFUNCTION(NetMulticast, Reliable)
	void MultiReload();

	void PlayFireAnimations() const;
	void SpawnTracer(const FHitResult& HitResult) const;
	void SpawnImpactParticles(const FHitResult& HitResult) const;

private:
	UFUNCTION()
	void OnRep_Equipped() const;
};
