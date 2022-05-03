// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MenuUi.generated.h"

class UButton;

/**
 * 
 */
UCLASS()
class SUPERGOONONLINE_API UMenuUi : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 publicConnections = 4, FString matchType = TEXT("FreeForAll"), FString lobbyPath = TEXT("/Game/ThirdPersonCPP/Maps/Lobby"));

protected:
	bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;


	//Callbacks for the multiplayer subsystem, they need to be UFUNCTIONS to be bound correctly in UE if they are dynamic
	//Look in the Supergoon Session Subsystem for what they are
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSession(const TArray<FOnlineSessionSearchResult>& sessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget) )
	UButton* HostButton;
	UPROPERTY(meta = (BindWidget) )
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTeardown();

	class USupergoonSessionsSubsystem* SupergoonSessionsSubsystem;

	int32 numPublicConnections = 4;
	FString MatchType = TEXT("FreeForAll");

	FString PathToLobby = "";
	
	
};
