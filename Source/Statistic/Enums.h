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
    CT_None,
};

USTRUCT(BlueprintType)
struct FSkillInfo
{
    GENERATED_BODY()

    UPROPERTY()
    float Damage;

    UPROPERTY()
    EClassType Element;

    UPROPERTY()
    float StunTime;

    UPROPERTY()
    FVector KnockbackDir;
};