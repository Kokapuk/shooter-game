#pragma once

#include "CoreMinimal.h"
#include "Delegates.h"
#include "SGWeaponDataAsset.h"
#include "Components/ActorComponent.h"
#include "SGWeaponComponent.generated.h"

class ASGCharacter;
class USGWeaponDataAsset;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="Weapon Component")
class SHOOTERGAME_API USGWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USGWeaponComponent();

	FOnFire OnFire;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure)
	bool HasAuthority() const { return GetOwner()->HasAuthority(); }

	UFUNCTION(BlueprintPure)
	bool IsLocallyControlled() const;

	UFUNCTION(BlueprintPure)
	bool IsOwnerLocalViewTarget() const;

	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Equip")
	void ServerEquip(USGWeaponDataAsset* Weapon);

	UFUNCTION(BlueprintPure)
	USGWeaponDataAsset* GetEquipped() const { return Equipped; }

	UFUNCTION(BlueprintPure)
	int32 GetRounds() const { return Rounds; }

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName="Fire")
	void CosmeticFire();

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName="Reload")
	void CosmeticReload();

	UFUNCTION(BlueprintPure)
	FVector GetFireDirection() const;

	UFUNCTION(BlueprintCallable, DisplayName="Reset", BlueprintAuthorityOnly)
	void AuthReset();

protected:
	UPROPERTY(EditDefaultsOnly)
	USoundBase* HitMarker;

	UPROPERTY(ReplicatedUsing=OnRep_Equipped)
	USGWeaponDataAsset* Equipped;

	float TimeToFire;

	UPROPERTY(Replicated)
	int32 Rounds;

	UPROPERTY(Replicated)
	uint8 bIsReloading : 1;

	UFUNCTION(BlueprintPure)
	bool CanFire() const;

	UFUNCTION(Server, Unreliable)
	void ServerFire(const FHitResult& HitResult);

	UFUNCTION(NetMulticast, Unreliable)
	void MultiFire(const FHitResult& HitResult);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, DisplayName="ResetRounds")
	virtual void AuthResetRounds();
	
	UFUNCTION(BlueprintPure)
	virtual bool CanReload() const;

	UFUNCTION(Server, Unreliable)
	void ServerReload();

	UFUNCTION(NetMulticast, Reliable)
	void MultiReload();

	UFUNCTION()
	void AuthFinishReload();

	void PlayFireAnimations() const;
	void SpawnTracer(const FHitResult& HitResult) const;
	void PlayImpactEffects(const FHitResult& HitResult) const;

	UFUNCTION(BlueprintCallable, BlueprintCosmetic, DisplayName="Play Hit Marker")
	void PlayHitMarker() const;

private:
	FTimerHandle ReloadingHandle;

	UFUNCTION()
	void OnRep_Equipped() const;
};
