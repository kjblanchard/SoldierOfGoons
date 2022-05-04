// Fill out your copyright notice in the Description page of Project Settings.


#include "SupergoonAnimInstance.h"

#include "SupergoonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void USupergoonAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	SupergoonCharacter = Cast<ASupergoonCharacter>(TryGetPawnOwner());
}

void USupergoonAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (SupergoonCharacter == nullptr)
	{
		SupergoonCharacter = Cast<ASupergoonCharacter>(TryGetPawnOwner());
	}
	if (SupergoonCharacter == nullptr)
		return;

	auto velocity = SupergoonCharacter->GetVelocity();
	velocity.Z = 0;

	Speed = velocity.Size();
	bIsInAir = SupergoonCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = SupergoonCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	bWeaponEquipped = SupergoonCharacter->IsWeaponEquipped();
	bIsCrouched = SupergoonCharacter->bIsCrouched;
	bIsAiming = SupergoonCharacter->IsAiming();


	//Offset yaw for strafing
	auto aimRotation = SupergoonCharacter->GetBaseAimRotation();
	auto movementRotation = UKismetMathLibrary::MakeRotFromX(SupergoonCharacter->GetVelocity());
	auto frameDeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(movementRotation, aimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, frameDeltaRot,DeltaSeconds, 6.f );
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = SupergoonCharacter->GetActorRotation();
	auto deltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const auto target = deltaRotation.Yaw / DeltaSeconds;
	const auto interp = FMath::FInterpTo(Lean, target, DeltaSeconds, 6.0f);
	Lean = FMath::Clamp(interp, -90.f, 90.f);
	
	
}
