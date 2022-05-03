// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "SoldierOfGoons/Character/SupergoonCharacter.h"
#include "SoldierOfGoons/Weapon/Weapon.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCombatComponent::EquipWeapon(AWeapon* weaponToEquip)
{
	if(SupergoonCharacter == nullptr || weaponToEquip == nullptr)
		return;
	EquippedWeapon = weaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	auto rightHandSocket = SupergoonCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if(rightHandSocket)
	{
		rightHandSocket->AttachActor(EquippedWeapon, SupergoonCharacter->GetMesh());
		EquippedWeapon->SetOwner(SupergoonCharacter);
	}

	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	
}



