#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SGAbilityComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Abstract, DisplayName="Ability Component")
class SHOOTERGAME_API USGAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USGAbilityComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure)
	virtual bool CanBeUtilized() const { return RemainingCooldownTime == 0.f; }

	UFUNCTION(BlueprintPure)
	float GetRemainingCooldown() const { return RemainingCooldownTime; }

	UFUNCTION(Server, Reliable, BlueprintCallable, DisplayName="Utilize")
	void ServerUtilize();

protected:
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime;

	UPROPERTY(Replicated)
	float RemainingCooldownTime;
};
