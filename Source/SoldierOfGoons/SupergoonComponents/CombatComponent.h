// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SOLDIEROFGOONS_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ASupergoonCharacter;
	void EquipWeapon(AWeapon* weaponToEquip);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	


protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);
	UFUNCTION(Server,Reliable)
	void ServerSetAiming(bool bAiming);

private:
	UPROPERTY(Replicated)
	AWeapon* EquippedWeapon;
	ASupergoonCharacter* SupergoonCharacter;

	UPROPERTY(Replicated)
	bool bIsAiming;

		
};
