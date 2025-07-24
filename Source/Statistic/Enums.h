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

UENUM(BlueprintType)
enum class EItemDropType : uint8
{
    Bottle,
    HealPotion_L,
    HealPotion_S,
    StaminaPotion_L,
    StaminaPotion_S,
    ManaPotion_L,
    ManaPotion_S,
    StoneCrystal,
    FireCrystal,
    IceCrystal,
};

UENUM(BlueprintType)
enum class EItemWorldType : uint8
{
    HP_Flower,
    MP_Flower,
    Stamina_Flower,
    Branch,
};

USTRUCT(BlueprintType)
struct FItemInventory
{
    GENERATED_BODY()

    // 아이템 종류별 개수
    UPROPERTY()
    TMap<EItemDropType, int32> ItemCounts;

    // 아이템 추가
    void AddItem(EItemDropType Type, int32 Amount = 1)
    {
        ItemCounts.FindOrAdd(Type) += Amount;
    }

    // 아이템 사용(감소)
    bool RemoveItem(EItemDropType Type, int32 Amount = 1)
    {
        int32* Count = ItemCounts.Find(Type);
        if (Count && *Count >= Amount)
        {
            *Count -= Amount;
            return true;
        }
        return false;
    }

    // 개수 조회
    int32 GetCount(EItemDropType Type) const
    {
        const int32* Count = ItemCounts.Find(Type);
        return Count ? *Count : 0;
    }

    // 포션류만 확률에 따라 하나 선택 (Bottle 82%, L 각각 1%, S 각각 5%)
    static EItemDropType GetRandomPotionType()
    {
        int32 Rand = FMath::RandRange(0, 99);

        if (Rand < 82)
            return EItemDropType::Bottle;
        else if (Rand < 83)
            return EItemDropType::HealPotion_L;
        else if (Rand < 84)
            return EItemDropType::ManaPotion_L;
        else if (Rand < 85)
            return EItemDropType::StaminaPotion_L;
        else if (Rand < 90)
            return EItemDropType::HealPotion_S;
        else if (Rand < 95)
            return EItemDropType::ManaPotion_S;
        else
            return EItemDropType::StaminaPotion_S;
    }
};