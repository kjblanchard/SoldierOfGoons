// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuUi.h"

#include "SupergoonSessionsSubsystem.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

void UMenuUi::MenuSetup(int32 publicConnections, FString matchType, FString lobbyPath)
{
	 PathToLobby = FString::Printf(TEXT("%s?listen"), *lobbyPath);
	numPublicConnections = publicConnections;
	MatchType = matchType;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	auto world = GetWorld();
	if (world)
	{
		auto playerController = world->GetFirstPlayerController();

		if (playerController)
		{
			//Configure the input mode for this
			FInputModeUIOnly inputModeData;
			inputModeData.SetWidgetToFocus(TakeWidget());
			inputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			playerController->SetInputMode(inputModeData);

			//Show your cursor
			playerController->SetShowMouseCursor(true);
		}
	}

	auto gameInstance = GetGameInstance();
	if (gameInstance)
	{
		SupergoonSessionsSubsystem = gameInstance->GetSubsystem<USupergoonSessionsSubsystem>();
	}

	//Bind all of our delegates to our subsystems delegates
	if (SupergoonSessionsSubsystem)
	{
		SupergoonSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		SupergoonSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSession);
		SupergoonSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		SupergoonSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		SupergoonSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenuUi::Initialize()
{
	//call super, and return false if it does
	if (!Super::Initialize())
		return false;

	//bind the functions to the buttons on clicked
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenuUi::HostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMenuUi::JoinButtonClicked);
	}

	return true;
}

void UMenuUi::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTeardown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenuUi::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString(TEXT("Session Created Successfully")));
		}
		auto world = GetWorld();
		if (world)
		{
			world->ServerTravel(PathToLobby);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString(TEXT("Session not Created Successfully")));
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMenuUi::OnFindSession(const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful)
{
	if (SupergoonSessionsSubsystem == nullptr)
		return;
	for (auto SearchResult : sessionResults)
	{
		FString matchType;
		SearchResult.Session.SessionSettings.Get(FName("MatchType"), matchType);

		if (matchType == MatchType)
		{
			SupergoonSessionsSubsystem->JoinSession(SearchResult);
			return;
		}
	}
	if(!bWasSuccessful || sessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenuUi::OnJoinSession(EOnJoinSessionCompleteResult::Type result)
{
	auto subsystem = IOnlineSubsystem::Get();
	if (subsystem)
	{
		auto sessionInterface = subsystem->GetSessionInterface();
		if (sessionInterface.IsValid())
		{
			FString address;
			auto resolvedProperly = sessionInterface->GetResolvedConnectString(NAME_GameSession, address);
			auto playerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (playerController)
			{
				playerController->ClientTravel(address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	if(result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

void UMenuUi::OnDestroySession(bool bWasSuccessful)
{
}

void UMenuUi::OnStartSession(bool bWasSuccessful)
{
}

void UMenuUi::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (SupergoonSessionsSubsystem)
	{
		SupergoonSessionsSubsystem->CreateSession(numPublicConnections, MatchType);
	}
}

void UMenuUi::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (SupergoonSessionsSubsystem)
	{
		SupergoonSessionsSubsystem->FindSessions(10000);
	}
}

void UMenuUi::MenuTeardown()
{
	RemoveFromParent();
	auto world = GetWorld();
	if (world)
	{
		auto playerController = world->GetFirstPlayerController();
		if (playerController)
		{
			FInputModeGameOnly inputModeData;
			playerController->SetInputMode(inputModeData);
			playerController->SetShowMouseCursor(false);
		}
	}
}
