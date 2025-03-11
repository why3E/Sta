// Copyright Epic Games, Inc. All Rights Reserved.

#include "StatisticGameMode.h"
#include "StatisticCharacter.h"
#include "UObject/ConstructorHelpers.h"

AStatisticGameMode::AStatisticGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
