// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Enums.h" // EClassType 포함
#include "ReceiveDamageInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UReceiveDamageInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STATISTIC_API IReceiveDamageInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ReceiveSkillHit(const struct FSkillInfo& Info, AActor* Causer) = 0;
};
