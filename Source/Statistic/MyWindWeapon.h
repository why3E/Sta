// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWeapon.h"
#include "MyWindCutter.h"
#include "MyWindWeapon.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API AMyWindWeapon : public AMyWeapon
{
	GENERATED_BODY()
public:
	AMyWindWeapon();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// 생성 및 발사 함수
	void SpawnWindCutter(FVector FireLocation);
	void ShootWindCutter();
	void SpawnWindSkill(FVector TargetLocation);

protected:
    // 효과음
    UPROPERTY(EditAnywhere, Category = "Sound", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USoundBase> WindCutterShootSound;

	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> WindCutterClass;
	
	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> WindSkillClass;

	UPROPERTY()
	TObjectPtr<class AMyWindCutter> TempWindCutter;


	FName WindCutterSocket;

private:
	FVector FireLocation;
};
