// Fill out your copyright notice in the Description page of Project Settings.


#include "SupergoonSessionsSubsystem.h"

#include "OnlineSubsystem.h"

USupergoonSessionsSubsystem::USupergoonSessionsSubsystem():
	CreateSessionCompleteDelegate(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnSessionCreateComplete)),
	FindSessionsCompleteDelegate(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(
		FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))

{
	auto subsystem = IOnlineSubsystem::Get();
	if (subsystem)
	{
		SessionInterface = subsystem->GetSessionInterface();
	}
}

void USupergoonSessionsSubsystem::CreateSession(int32 numMaxSessions, FString matchType)
{
	if (!SessionInterface.IsValid())
		return;

	auto existingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	//Destroy a session if it exists already
	if (existingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = numMaxSessions;
		LastMatchType = matchType;
		DestroySession();
	}


	//Add our delegate to the delegate list in the online session interface
	// So, The session interfaces calls the delegate, which we bound to our Function in the ctor initializer list
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());

	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = numMaxSessions;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->bUseLobbiesIfAvailable = true;
	LastSessionSettings->BuildUniqueId = 1;

	//Set the match type we are creating for the matchtype
	LastSessionSettings->Set(FName("MatchType"), matchType,
	                         EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	auto sessionCreatedSuccessfully = SessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(),
	                                                                  NAME_GameSession, *LastSessionSettings);
	if (!sessionCreatedSuccessfully)
	{
		//Remove the delegate handle from the session interface if the session wasn't created properly
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void USupergoonSessionsSubsystem::FindSessions(int32 maxSessions)
{
	if (!SessionInterface.IsValid())
		return;
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = maxSessions;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	auto wasSessionsFound = SessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(),
	                                                       LastSessionSearch.ToSharedRef());
	if (!wasSessionsFound)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), wasSessionsFound);
	}
}

void USupergoonSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& sessionResult)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		JoinSessionCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	auto joinSessionSuccessful = SessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(),
	                                                           NAME_GameSession, sessionResult);
	if (!joinSessionSuccessful)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void USupergoonSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		DestroySessionCompleteDelegate);
	auto sessionDestroyed = SessionInterface->DestroySession(NAME_GameSession);
	if (!sessionDestroyed)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void USupergoonSessionsSubsystem::StartSession()
{
}

//Callbacks

void USupergoonSessionsSubsystem::OnSessionCreateComplete(FName sessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MultiplayerOnCreateSessionComplete.Broadcast(true);
	}
}

void USupergoonSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() == 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), bWasSuccessful);
		return;
	}
	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void USupergoonSessionsSubsystem::OnJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	MultiplayerOnJoinSessionComplete.Broadcast(result);
}

void USupergoonSessionsSubsystem::OnDestroySessionComplete(FName sessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		if (bWasSuccessful && bCreateSessionOnDestroy)
		{
			bCreateSessionOnDestroy = false;
			CreateSession(LastNumPublicConnections, LastMatchType);
		}
		MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
	}
}

void USupergoonSessionsSubsystem::OnStartSessionComplete(FName sessionName, bool bWasSuccessful)
{
}
