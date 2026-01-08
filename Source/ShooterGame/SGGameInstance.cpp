#include "SGGameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"

void USGGameInstance::Init()
{
	Super::Init();

	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	check(SessionInterface.IsValid())

	InviteAcceptedHandle =
		SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
			FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &USGGameInstance::OnSessionInviteAccepted));

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "INIT");
}

void USGGameInstance::LockSession()
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	check(SessionInterface.IsValid())

	FOnlineSessionSettings* Settings = SessionInterface->GetSessionSettings(NAME_GameSession);
	check(Settings)

	Settings->bAllowInvites = false;
	Settings->bAllowJoinInProgress = false;
	Settings->bAllowJoinViaPresence = false;
	Settings->bShouldAdvertise = false;

	SessionInterface->UpdateSession(NAME_GameSession, *Settings, true);
}

void USGGameInstance::OnSessionInviteAccepted(bool bWasSuccessful, int32 ControllerId,
                                              TSharedPtr<const FUniqueNetId> UserId,
                                              const FOnlineSessionSearchResult& InviteResult)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
	                                 FString::Printf(
		                                 TEXT("OnSessionInviteAccepted bWasSuccessful:%d InviteResult.IsValid():%d"),
		                                 bWasSuccessful,
		                                 InviteResult.IsValid()));

	if (!bWasSuccessful || !InviteResult.IsValid()) return;

	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	check(SessionInterface.IsValid())

	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &USGGameInstance::OnJoinSessionComplete));
	SessionInterface->JoinSession(ControllerId, NAME_GameSession, InviteResult);
}

void USGGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	check(SessionInterface.IsValid())

	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
	                                 FString::Printf(
		                                 TEXT(
			                                 "OnJoinSessionComplete Result == EOnJoinSessionCompleteResult::Success:%d"),
		                                 Result == EOnJoinSessionCompleteResult::Success));

	if (Result != EOnJoinSessionCompleteResult::Success) return;

	FString ConnectString;
	bool bWasSuccessful = SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectString);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
	                                 FString::Printf(
		                                 TEXT("OnJoinSessionComplete bWasSuccessful:%d"),
		                                 bWasSuccessful));
	if (!bWasSuccessful) return;
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, ConnectString);

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	check(IsValid(PlayerController))
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
}
