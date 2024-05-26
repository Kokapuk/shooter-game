#pragma once

#include "CoreMinimal.h"
#include "SGAbilityComponent.h"
#include "SGFlashAbilityComponent.generated.h"

class ASGFlash;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), DisplayName="Flash Ability Component", Blueprintable)
class SHOOTERGAME_API USGFlashAbilityComponent : public USGAbilityComponent
{
	GENERATED_BODY()

public:
	USGFlashAbilityComponent();

	virtual bool CanBeUtilized() const override { return Super::CanBeUtilized() && LineTrace().bBlockingHit; }
	virtual void ServerUtilize_Implementation() override;

protected:
	FHitResult LineTrace() const;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ASGFlash> FlashClass;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0.f))
	float MaxDeployDistance;
};
