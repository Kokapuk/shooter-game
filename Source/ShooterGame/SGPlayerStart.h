#pragma once

#include "CoreMinimal.h"
#include "SGPlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "SGPlayerStart.generated.h"

class ASGPlayerController;
enum class ETeam : uint8;

UCLASS()
class SHOOTERGAME_API ASGPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:
	ASGPlayerStart(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
	void AuthOccupy(APlayerController* Player);
	
	UFUNCTION(BlueprintPure)
	ETeam GetTeam() const { return Team; }

	UFUNCTION(BlueprintPure)
	uint8 IsOccupied() const { return IsValid(OccupiedBy); }

	UFUNCTION(BlueprintPure)
	APlayerController* GetOccupiedBy() const { return OccupiedBy; }

protected:
	UPROPERTY(EditInstanceOnly)
	ETeam Team;

	UPROPERTY(VisibleInstanceOnly)
	APlayerController* OccupiedBy;
};
