#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGAbilityComponent.generated.h"


class USGAbilityDataAsset;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract, DisplayName="Ability Component")
class SHOOTERGAME_API USGAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USGAbilityComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure)
	virtual bool CanBeUtilized() const;

	UFUNCTION(BlueprintPure)
	float GetCooldown() const { return Cooldown; }

	UFUNCTION(BlueprintPure)
	float GetRemainingCooldown() const;

	UFUNCTION(BlueprintPure)
	USGAbilityDataAsset* GetAbilityDataAsset() const { return AbilityDataAsset; }

	UFUNCTION(BlueprintCallable)
	void Utilize();

	UFUNCTION(BlueprintCallable, DisplayName="Reset", BlueprintAuthorityOnly)
	void AuthReset();

	UFUNCTION(Client, Reliable)
	void ClientReset();

protected:
	virtual void CosmeticUtilize();

	UFUNCTION(Server, Reliable)
	void ServerUtilize();

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.f))
	float Cooldown;

	UPROPERTY(EditDefaultsOnly)
	USGAbilityDataAsset* AbilityDataAsset;

private:
	UPROPERTY(Replicated)
	float CoolsDownOn;
};
