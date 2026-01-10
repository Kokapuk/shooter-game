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
}

void USGGameInstance::LockSession()
{
	const UWorld* World = GetWorld();
	if (World->IsPlayInEditor()) return;

	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(World);
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
	if (!bWasSuccessful || !InviteResult.IsValid()) return;

	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	check(SessionInterface.IsValid())

	JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &USGGameInstance::OnJoinSessionComplete));
	SessionInterface->JoinSession(ControllerId, NAME_GameSession, InviteResult);
}

void USGGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	check(SessionInterface.IsValid())

	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);

	if (Result != EOnJoinSessionCompleteResult::Success) return;

	FString ConnectString;
	if (!SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectString)) return;

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	check(IsValid(PlayerController))
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
}
