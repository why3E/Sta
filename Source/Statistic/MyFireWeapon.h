// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWeapon.h"
#include "MyFireBall.h"
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
	void SpawnFireBall();
	void ShootFireBall();
	protected:
	// 타겟 위치를 구하기 위한 함수
	void SetFireLocation();

	// 에너지볼 클래스
	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> FireBallClass;

	// 현재 스폰된 에너지볼 객체
	UPROPERTY()
	TObjectPtr<class AMyFireBall> TempFireBall;

	// 에너지 볼을 스폰하기 위한 소켓
	FName FireBallSocket;

private:
	FVector FireLocation;
};
