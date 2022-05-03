// Fill out your copyright notice in the Description page of Project Settings.


#include "SupergoonAnimInstance.h"

#include "SupergoonCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	if(SupergoonCharacter == nullptr)
		return;

	auto velocity = SupergoonCharacter->GetVelocity();
	velocity.Z = 0;

	Speed = velocity.Size();
	bIsInAir = SupergoonCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = SupergoonCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	bWeaponEquipped = SupergoonCharacter->IsWeaponEquipped();
	bIsCrouched = SupergoonCharacter->bIsCrouched;
}
