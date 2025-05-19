// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWeapon.h"
#include "MyStoneWave.h"
#include "MyStoneWeapon.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API AMyStoneWeapon : public AMyWeapon
{
	GENERATED_BODY()
public:
	AMyStoneWeapon();
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
public:
	// 생성 및 발사 함수
	void SpawnStoneWave(FVector FireLocation);
	void SpawnStoneSkill(FVector TargetLocation);
protected:
	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> StoneWaveClass;
};
