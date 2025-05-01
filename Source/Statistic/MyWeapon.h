// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterStat.h"
#include "NiagaraComponent.h"
#include "MyWeapon.generated.h"

UENUM(BlueprintType)	
enum class EWeaponType : uint8
{
	WT_Wind,
	WT_Fire,
	WT_Stone,
	WT_Ice	
};

UCLASS()
class STATISTIC_API AMyWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyWeapon();

public:
	FORCEINLINE EWeaponType GetWeaponType() { return WeaponType; }
	
public:
	void EquipWeapon(ACharacter* Player);
	void DrawWeapon(USkeletalMeshComponent* Mesh);
	void SheatheWeapon(USkeletalMeshComponent* Mesh);

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UNiagaraComponent> WeaponEffect; // 나이아가라 컴포넌트로 변경


	UPROPERTY(VisibleAnywhere, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> WeaponCollision;

	EWeaponType WeaponType;

	FCharacterStat WeaponStat;

	FName BaseSocketName;

	UPROPERTY()
    ACharacter* OwnerCharacter;
};
