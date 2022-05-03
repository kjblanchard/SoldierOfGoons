// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "SoldierOfGoons/Character/SupergoonCharacter.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	//Makes it so that the variables on this can actually be replicated.
	bReplicates = true;
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon Mesh");
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere = CreateDefaultSubobject<USphereComponent>("Area Sphere");
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>("Pickup Widget");
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}
	if (PickupWidget)
		PickupWidget->SetVisibility(false);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor,
                              UPrimitiveComponent* otherComponent, int32 otherBodyIndex, bool bFromSweep,
                              const FHitResult& sweepResult)
{
	auto supergoonCharacter = Cast<ASupergoonCharacter>(otherActor);
	if (supergoonCharacter && PickupWidget)
	{
		supergoonCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor,
                                 UPrimitiveComponent* otherComponent, int32 otherBodyIndex)
{
	auto supergoonCharacter = Cast<ASupergoonCharacter>(otherActor);
	if (supergoonCharacter && PickupWidget)
	{
		supergoonCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetWeaponState(EWeaponState state)
{
	WeaponState = state;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		break;
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

//Register the things that we want replicated
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}
