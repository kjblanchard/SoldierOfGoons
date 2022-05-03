// Fill out your copyright notice in the Description page of Project Settings.


#include "SupergoonCharacter.h"

#include <string>

#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetStringLibrary.h"
#include "Net/UnrealNetwork.h"
#include "SoldierOfGoons/SupergoonComponents/CombatComponent.h"
#include "SoldierOfGoons/Weapon/Weapon.h"

ASupergoonCharacter::ASupergoonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>("OverheadWidget");
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>("Combat Component");
	//To make variables on this able to be replicated, you need the following, it basically registers it with the server
	CombatComponent->SetIsReplicated(true);
}

void ASupergoonCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//Only replicate to the owner.
	DOREPLIFETIME_CONDITION(ASupergoonCharacter, OverlappedWeapon, COND_OwnerOnly);
}

//After all the components are initialized
void ASupergoonCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->SupergoonCharacter = this;
	}
}

void ASupergoonCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetMovementComponent()->NavAgentProps.bCanCrouch = true;
}

void ASupergoonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ASupergoonCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASupergoonCharacter::CrouchButtonPressed);
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASupergoonCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASupergoonCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ASupergoonCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ASupergoonCharacter::LookUp);
}


void ASupergoonCharacter::MoveForward(float value)
{
	if (Controller != nullptr && value != 0.f)
	{
		//This gets the direction of our controller, so it is not based on our character.
		const auto yawRotation = FRotator(0, Controller->GetControlRotation().Yaw, 0);
		const auto direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);

		AddMovementInput(direction, value);
	}
}

void ASupergoonCharacter::MoveRight(float value)
{
	if (Controller != nullptr && value != 0.f)
	{
		//This gets the direction of our controller, so it is not based on our character.
		const auto yawRotation = FRotator(0, Controller->GetControlRotation().Yaw, 0);
		const auto direction = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(direction, value);
	}
}

void ASupergoonCharacter::EquipButtonPressed()
{
	//If we are the server/host handle it
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappedWeapon);
		}
		else
		{
			//Note we call this without implementation
			ServerEquipButtonPressed();
		}
	}
}

//UE will do some work behind the scenes, so the definition must have a _Implementation suffix as UE will maeke the prefix
void ASupergoonCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappedWeapon);
	}
}

void ASupergoonCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ASupergoonCharacter::Turn(float value)
{
	AddControllerYawInput(value);
}

void ASupergoonCharacter::LookUp(float value)
{
	AddControllerPitchInput(value);
}

/**
 * @brief The function that is called on all CLIENTS (not server) when its binding is changed ON THE SERVER.
 * @param lastWeapon The last weapon that was overlapped before this was called
 */
void ASupergoonCharacter::OnRep_OverlappingWeapon(AWeapon* lastWeapon)
{
	if (OverlappedWeapon)
	{
		OverlappedWeapon->ShowPickupWidget(true);
	}
	if (lastWeapon)
	{
		lastWeapon->ShowPickupWidget(false);
	}
}


void ASupergoonCharacter::SetOverlappingWeapon(AWeapon* weapon)
{
	//Handle if WE ARE THE SERVER/HOST since replication does not send notifys to the server, only clients.

	if (IsLocallyControlled() && OverlappedWeapon)
		OverlappedWeapon->ShowPickupWidget(false);
	OverlappedWeapon = weapon;
	if (IsLocallyControlled() && OverlappedWeapon)
	{
		OverlappedWeapon->ShowPickupWidget(true);
	}
}

bool ASupergoonCharacter::IsWeaponEquipped()
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

void ASupergoonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
