#pragma once

#include "CoreMinimal.h"
#include "ShooterGame/SGWeaponComponent.h"
#include "SGRouletteWeaponComponent.generated.h"

UCLASS()
class SHOOTERGAME_API USGRouletteWeaponComponent : public USGWeaponComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetRounds(const int32 NewRounds);

protected:
	virtual void AuthResetRounds() override;
	virtual bool CanReload() const override;
	
};
