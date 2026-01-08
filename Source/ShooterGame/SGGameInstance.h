#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SGGameInstance.generated.h"

UCLASS()
class SHOOTERGAME_API USGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	
	UFUNCTION(BlueprintCallable)
	void LockSession();

private:
	FDelegateHandle InviteAcceptedHandle;
	FDelegateHandle JoinSessionCompleteHandle;

	void OnSessionInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId,
	                             const FOnlineSessionSearchResult& InviteResult);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
