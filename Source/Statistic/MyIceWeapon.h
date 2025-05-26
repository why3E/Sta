// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWeapon.h"
#include "MyIceArrow.h"
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
public:
	void SetAiming();
	// 생성 및 발사 함수
	void ShootIceArrow(FVector FirePoint);

	void SpawnIceSkill(FVector Location, FRotator Rotation);

protected:
	// 클래스
	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> IceArrowClass;
	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> IceSkillClass;

	UPROPERTY()
	TObjectPtr<class AMyIceArrow> TempIceArrow;


private:
	FVector FireLocation;
private:
	bool bIsAiming = false;
};
