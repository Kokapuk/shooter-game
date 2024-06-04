#pragma once

#include "CoreMinimal.h"
#include "ShooterGame/SGGameMode.h"
#include "SGVersusGameMode.generated.h"

UCLASS()
class SHOOTERGAME_API ASGVersusGameMode : public ASGGameMode
{
	GENERATED_BODY()

public:
	ASGVersusGameMode();
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
	virtual void StartMatch() override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintPure)
	bool IsFriendlyFireAllowed() const;

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void FinishRound();

protected:
	UPROPERTY(EditDefaultsOnly)
	uint8 bIsFriendlyFireAllowed : 1;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=1))
	int32 RoundsToWin;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=1.f))
	float PostRoundTime;

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void ResetPlayers();
	
	UFUNCTION()
	void StartNewRound();
};
