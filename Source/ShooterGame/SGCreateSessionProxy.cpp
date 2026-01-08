#include "SGCreateSessionProxy.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Interfaces/OnlineSessionInterface.h"

USGCreateSessionProxy* USGCreateSessionProxy::CreateSession(const int32 PublicConnections)
{
	USGCreateSessionProxy* Proxy = NewObject<USGCreateSessionProxy>();
	Proxy->NumPublicConnections = PublicConnections;
	
	return Proxy;
}

void USGCreateSessionProxy::Activate()
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());

	if (!SessionInterface.IsValid())
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
		return;
	}

	FOnlineSessionSettings Settings;
	Settings.bIsLANMatch = false;
	Settings.bUsesPresence = true;
	Settings.bAllowInvites = true;
	Settings.bAllowJoinViaPresence = true;
	Settings.bAllowJoinViaPresenceFriendsOnly = true;
	Settings.bShouldAdvertise = true;
	Settings.NumPublicConnections = NumPublicConnections;
	Settings.bUseLobbiesIfAvailable = true;

	CreateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &USGCreateSessionProxy::OnCreateSessionComplete));

	if (!SessionInterface->CreateSession(0, NAME_GameSession, Settings))
	{
		OnFailure.Broadcast();
		SetReadyToDestroy();
	}
}

void USGCreateSessionProxy::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
	}

	if (bWasSuccessful) OnSuccess.Broadcast();
	else OnFailure.Broadcast();

	SetReadyToDestroy();
}
