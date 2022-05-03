// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	auto numberOfPlayers = GameState.Get()->PlayerArray.Num();
	if(numberOfPlayers >= 2)
	{
		auto world = GetWorld();
		if(world)
		{
			bUseSeamlessTravel = true;
			world->ServerTravel(FString("/Game/Maps/SupergoonMap?listen"));
		}
		
	}
}
