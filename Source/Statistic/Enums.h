#pragma once

#include "CoreMinimal.h"
#include "Enums.generated.h"

UENUM(BlueprintType)
enum class EClassType : uint8
{
    CT_Wind,
    CT_Fire,
    CT_Ice,
    CT_Stone,
};