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

UENUM(BlueprintType)
enum class BossTypes : uint8 {
	IceGiant,
	WoodGiant
};

USTRUCT(BlueprintType)
struct FItemInventory
{
    GENERATED_BODY()

    // 아이템 종류별 개수
    UPROPERTY()
    TMap<EItemDropType, int32> DropItemCounts;
    UPROPERTY()
    TMap<EItemWorldType, int32> WorldItemCounts;

    FItemInventory()
    {
        const UEnum* DropEnum = StaticEnum<EItemDropType>();
        if (DropEnum)
        {
            for (int32 i = 0; i < DropEnum->NumEnums() - 1; ++i)
            {
                DropItemCounts.Add(static_cast<EItemDropType>(i), 0);
            }
        }

        const UEnum* WorldEnum = StaticEnum<EItemWorldType>();
        if (WorldEnum)
        {
            for (int32 i = 0; i < WorldEnum->NumEnums() - 1; ++i)
            {
                WorldItemCounts.Add(static_cast<EItemWorldType>(i), 0);
            }
        }

    } 

    // 아이템 추가
    void AddItem(EItemDropType Type, int32 Amount = 1)
    {
        int32& Count = DropItemCounts.FindOrAdd(Type);
        Count += Amount;

        // 로그 출력
        UE_LOG(LogTemp, Log, TEXT("[인벤토리] %s 아이템 %d개 추가됨. 현재 개수: %d"),
        *UEnum::GetValueAsString(Type),
        Amount,
        Count);
    }

    // 아이템 사용(감소)
    bool RemoveItem(EItemDropType Type, int32 Amount = 1)
    {
        int32* Count = DropItemCounts.Find(Type);
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
        const int32* Count = DropItemCounts.Find(Type);
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


    void AddItem(EItemWorldType Type, int32 Amount = 1)
    {
        int32& Count = WorldItemCounts.FindOrAdd(Type);
        Count += Amount;

    // 로그 출력
        UE_LOG(LogTemp, Log, TEXT("[인벤토리] %s 아이템 %d개 추가됨. 현재 개수: %d"),
        *UEnum::GetValueAsString(Type),
        Amount,
        Count);
    }

    // 아이템 사용(감소)
    bool RemoveItem(EItemWorldType Type, int32 Amount = 1)
    {
        int32* Count = WorldItemCounts.Find(Type);
        if (Count && *Count >= Amount)
        {
            *Count -= Amount;
            return true;
        }
        return false;
    }

    // 개수 조회
    int32 GetCount(EItemWorldType Type) const
    {
        const int32* Count = WorldItemCounts.Find(Type);
        return Count ? *Count : 0;
    }
};