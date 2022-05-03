// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UOverheadWidget::ShowPlayerNetRole(APawn* inPawn)
{
	auto localRole = inPawn->GetLocalRole();
	FString role;
	switch (localRole)
	{
	case ENetRole::ROLE_Authority:
		role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		role = FString("None");
		break;
	}
	auto localRoleString = FString::Printf(TEXT("Local role: %s"), *role);
	SetDisplayText(localRoleString);
	
	
}

void UOverheadWidget::SetDisplayText(FString textToDisplay)
{
		DisplayText->SetText(FText::FromString(textToDisplay));
}
