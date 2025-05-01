// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "AnimationAttackInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAnimationAttackInterface : public UInterface
{
	GENERATED_BODY()
};

class STATISTIC_API IAnimationAttackInterface
{
	GENERATED_BODY()
public:
	// 기본 공격 성공/실패 여부 체크
	virtual void BaseAttackCheck() = 0;
};
