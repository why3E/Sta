// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWeapon.h"
#include "MyFireBall.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "MyFireWeapon.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API AMyFireWeapon : public AMyWeapon
{
	GENERATED_BODY()
public:
	AMyFireWeapon();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// 생성 및 발사 함수
	void SpawnFireBall(FVector ImpactPoint);
	void ShootFireBall();
	void SpawnFireSkill(FVector TargetLocation, FRotator TargetRotation);
protected:
	// 클래스
	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> FireBallClass;
	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> FireSkillClass;
	// 현재 스폰된 객체
	UPROPERTY()
	TObjectPtr<class AMyFireBall> TempFireBall;

	// 에너지 볼을 스폰하기 위한 소켓
	FName FireBallSocket;

private:
	FVector FireLocation;
protected:
    // 효과음
    UPROPERTY(EditAnywhere, Category = "Sound", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USoundBase> FireBallShootSound;
};
