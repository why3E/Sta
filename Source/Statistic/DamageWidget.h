#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageWidget.generated.h"

/**
 * 데미지 표시용 위젯 클래스
 */
UCLASS()
class STATISTIC_API UDamageWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    // 데미지를 표시할 텍스트 블록
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TextBlock_Damage;

    // 일반 데미지 애니메이션
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    class UWidgetAnimation* damageAnimNormalDamage;

public:
    // 데미지 텍스트를 설정하는 함수
    UFUNCTION()
    void SetDamageText(FString Damage);

    // 위젯 숨기기
    UFUNCTION()
    void HideWidget();

    // 위젯 보이기
    UFUNCTION()
    void ShowWidget();

    // 일반 데미지 애니메이션 재생
    UFUNCTION()
    void PlayNormalDamageAnimation(float Damage);

protected:
    // 위젯 초기화
    virtual void NativeConstruct() override;
};