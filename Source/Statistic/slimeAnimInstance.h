// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyCharacterAnimInstance.h"
#include "slimeAnimInstance.generated.h"


UCLASS()
class STATISTIC_API UslimeAnimInstance : public UEnemyCharacterAnimInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Move)
		float speedX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Move)
		float speedY;

protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

};
