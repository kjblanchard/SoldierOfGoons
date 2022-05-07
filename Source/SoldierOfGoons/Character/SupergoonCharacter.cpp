// Fill out your copyright notice in the Description page of Project Settings.


#include "SupergoonCharacter.h"

#include <string>

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
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
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
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
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASupergoonCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASupergoonCharacter::AimButtonReleased);
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
			//TODO fix this
			//Doing this currently as something is broken..
			// CombatComponent->EquipWeapon(OverlappedWeapon);
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
	UE_LOG(LogTemp, Warning, TEXT("Implementation going, overlap is: %s "), OverlappedWeapon);
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

void ASupergoonCharacter::AimButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void ASupergoonCharacter::AimButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
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

void ASupergoonCharacter::AimOffset(float deltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr)
		return;
	auto velocity = GetVelocity();
	velocity.Z = 0;
	auto speed = velocity.Size();
	auto bIsInAir = GetCharacterMovement()->IsFalling();

	//Standing still and not jumping
	if (speed == 0.f && !bIsInAir)
	{
		auto currentAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		FRotator deltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(currentAimRotation, StartingAimRotation);
		AO_Yaw = deltaAimRotation.Yaw;
		bUseControllerRotationYaw = false;
	}
	if (speed > 0 || bIsInAir)
	{
		StartingAimRotation = FRotator(0, GetBaseAimRotation().Yaw, 0);
		AO_Yaw = 0;
		bUseControllerRotationYaw = true;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90 && !IsLocallyControlled())
	{
		//Map Pictch from 270-360 to -90 -0 (Happens cause UE will compress info (doubles) when sending to server for efficiency
		FVector2d InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
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

bool ASupergoonCharacter::IsAiming()
{
	return CombatComponent && CombatComponent->bIsAiming;
}

void ASupergoonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	AimOffset(DeltaTime);
}
