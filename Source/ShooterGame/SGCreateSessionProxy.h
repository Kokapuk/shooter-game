#pragma once

#include "CoreMinimal.h"
#include "Net/OnlineBlueprintCallProxyBase.h"
#include "SGCreateSessionProxy.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCreateSessionResult);

UCLASS()
class USGCreateSessionProxy : public UOnlineBlueprintCallProxyBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FCreateSessionResult OnSuccess;

	UPROPERTY(BlueprintAssignable)
	FCreateSessionResult OnFailure;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "SG|Online|Session")
	static USGCreateSessionProxy* CreateSession(const int32 PublicConnections);

	virtual void Activate() override;

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	int32 NumPublicConnections;
	FDelegateHandle CreateHandle;
};