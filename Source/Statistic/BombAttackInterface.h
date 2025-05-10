// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Enums.h"
#include "BombAttackInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UBombAttackInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STATISTIC_API IBombAttackInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void MixBombAttack(EClassType MixType, unsigned short skill_id) = 0;
};
