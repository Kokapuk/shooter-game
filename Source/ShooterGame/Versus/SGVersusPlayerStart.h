#pragma once

#include "CoreMinimal.h"
#include "ShooterGame/Enums.h"
#include "ShooterGame/SGPlayerStart.h"
#include "SGVersusPlayerStart.generated.h"

UCLASS()
class SHOOTERGAME_API ASGVersusPlayerStart : public ASGPlayerStart
{
	GENERATED_BODY()

public:
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
