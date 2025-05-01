// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyWeapon.h"
#include "UObject/Interface.h"
#include "AnimationWeaponInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UAnimationWeaponInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class STATISTIC_API IAnimationWeaponInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual class AMyWeapon* GetWeapon() = 0;
};
