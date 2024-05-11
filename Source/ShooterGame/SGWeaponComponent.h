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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Equip")
	void ServerEquip(USGWeaponDataAsset* Weapon);

	UFUNCTION(BlueprintPure)
	USGWeaponDataAsset* GetEquipped() const { return Equipped; }

	UFUNCTION(BlueprintPure)
	int32 GetRounds() const { return Rounds; }

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Start Fire")
	void ServerStartFire();

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Stop Fire")
	void ServerStopFire();

	UFUNCTION(BlueprintCallable, DisplayName="Fire", BlueprintAuthorityOnly)
	void AuthFire();

	UFUNCTION(Server, Unreliable, BlueprintCallable, DisplayName="Reload")
	void ServerReload();

	UFUNCTION(BlueprintPure)
	FVector GetFireDirection() const;

	UFUNCTION(BlueprintCallable, DisplayName="Reset", BlueprintAuthorityOnly)
	void AuthRest();

protected:
	UPROPERTY(ReplicatedUsing=OnRep_Equipped)
	USGWeaponDataAsset* Equipped;

	uint8 bIsAutomaticallyFiring : 1;
	float TimeToFire;
	float ShootingError;

	UPROPERTY(Replicated)
	int32 Rounds;

	uint8 bIsReloading : 1;

	UFUNCTION(NetMulticast, Unreliable)
	void MultiFire(const FHitResult& HitResult);

	UFUNCTION(NetMulticast, Reliable)
	void MultiReload();

	UFUNCTION()
	void AuthFinishReload();

	void PlayFireAnimations() const;
	void SpawnTracer(const FHitResult& HitResult) const;
	void PlayImpactEffects(const FHitResult& HitResult) const;

private:
	FTimerHandle ReloadingHandle;
	
	UFUNCTION()
	void OnRep_Equipped() const;
};
