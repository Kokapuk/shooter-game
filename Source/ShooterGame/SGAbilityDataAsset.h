#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SGAbilityDataAsset.generated.h"

class USGAbilityComponent;

UCLASS()
class SHOOTERGAME_API USGAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<USGAbilityComponent> AbilityClass;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	UTexture2D* Icon;
};
