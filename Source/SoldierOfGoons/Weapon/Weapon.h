// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState: uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_Max UMETA(DisplayName = "Max"),
	
};

UCLASS()
class SOLDIEROFGOONS_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShowWidget);

protected:
	virtual void BeginPlay() override;
	//We are binding this to a delegate, so that is why it needs this specific signature
	//You can find this Oncomponentbeginoverlap with f12
	UFUNCTION()
	
	virtual void OnSphereOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor,
		UPrimitiveComponent* otherComponent, int32 otherBodyIndex,
		bool bFromSweep, const FHitResult& sweepResult);

	UFUNCTION()
		virtual void OnSphereEndOverlap(UPrimitiveComponent* overlappedComponent, AActor* otherActor,
                                		UPrimitiveComponent* otherComponent, int32 otherBodyIndex
			);

public:
	FORCEINLINE void SetWeaponState(EWeaponState state);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere;}

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	//Detect overlap with characters so that they can pick it up
	class USphereComponent* AreaSphere;
	
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;


	
	

};
