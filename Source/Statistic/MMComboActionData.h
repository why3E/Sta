#pragma once

#include "CoreMinimal.h"
#include "Enums.h" // EClassType 헤더 포함
#include "MMComboActionData.generated.h"

UCLASS()
class STATISTIC_API UMMComboActionData : public UDataAsset
{
    GENERATED_BODY()

public:
    UMMComboActionData();

    // 현재 클래스 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
    EClassType ClassType;

    // 몽타주 섹션 이름 (접두사)
    UPROPERTY(EditAnywhere, Category = "Combo")
    FString SectionPrefix;

    // 재생 속도
    UPROPERTY(EditAnywhere, Category = "Combo")
    float FrameRate;

    // 최대 가능 콤보 수
    UPROPERTY(EditAnywhere, Category = "Combo")
    uint8 MaxComboCount;

    // 콤보별 다음 콤보로 넘어가기 위한 입력 프레임 정보
    UPROPERTY(EditAnywhere, Category = "Combo")
    TArray<float> ComboFrame;
};
