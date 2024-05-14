#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SGGameMode.generated.h"

class APlayerStart;

UCLASS()
class SHOOTERGAME_API ASGGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASGGameMode();

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
	virtual void StartMatch() override;
	virtual void EndMatch() override;
	virtual void Logout(AController* Exiting) override;

	bool IsFriendlyFireAllowed() const { return bIsFriendlyFireAllowed; }

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void FinishRound();

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void ResetPlayers();

protected:
	static void ShufflePlayerStarts(TArray<AActor*>& PlayerStarts);

	UPROPERTY(EditDefaultsOnly)
	uint8 bIsFriendlyFireAllowed : 1;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin=1))
	int32 RoundsToWin;
};
