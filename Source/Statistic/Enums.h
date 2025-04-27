#pragma once

#include "CoreMinimal.h"
#include "Enums.generated.h"

UENUM(BlueprintType)
enum class EClassType : uint8
{
    CT_Wind,    // 초보자
    CT_Fire,     // 전사
    CT_Ice,      // 궁수
    CT_Stone,        // 마법사
};