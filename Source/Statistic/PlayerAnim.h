// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnim.generated.h"

UCLASS()
class STATISTIC_API UPlayerAnim : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Move)
		float speedX;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Move)
		float speedY;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Jump)
		bool isInAir;
protected:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

};
