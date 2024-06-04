#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SGGameMode.generated.h"

class ASGPlayerState;

UCLASS()
class SHOOTERGAME_API ASGGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ASGGameMode();

	virtual bool MustSpectate_Implementation(APlayerController* NewPlayerController) const override;
	virtual void StartMatch() override;
	virtual void EndMatch() override;

protected:
	static void ShufflePlayerStarts(TArray<AActor*>& PlayerStarts);

	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable)
	void ResetPlayer(ASGPlayerState* Player);

	UFUNCTION()
	void HandlePlayerKill(ASGPlayerState* Killer, ASGPlayerState* Victim, bool bIsHeadshot);
};
