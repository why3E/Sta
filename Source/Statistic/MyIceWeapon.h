// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWeapon.h"
#include "MyIceWeapon.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API AMyIceWeapon : public AMyWeapon
{
	GENERATED_BODY()
public:
	AMyIceWeapon();
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
protected:
    // 아이스 무기 스태틱 메시
    UPROPERTY(VisibleAnywhere, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UStaticMeshComponent> WeaponMesh;

	FName IceSocket;
};
