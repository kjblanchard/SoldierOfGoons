// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class SOLDIEROFGOONS_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;
	

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* inPawn);
	void SetDisplayText(FString textToDisplay);
	
};
