// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ImpactPointInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UImpactPointInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STATISTIC_API IImpactPointInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual FVector GetCurrentImpactPoint() = 0;
	virtual FRotator GetCurrentImpactRot() = 0;
	virtual FVector GetFireLocation() = 0;
};
