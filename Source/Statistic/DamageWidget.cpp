#include "DamageWidget.h"
#include "Components/TextBlock.h"

void UDamageWidget::NativeConstruct()
{
    Super::NativeConstruct();
    HideWidget();
}

void UDamageWidget::SetDamageText(FString Damage)
{
    if (TextBlock_Damage)
    {
        // 전달받은 데미지 문자열을 텍스트로 설정
        TextBlock_Damage->SetText(FText::FromString(Damage));
    }
}

void UDamageWidget::HideWidget()
{
    if (TextBlock_Damage)
    {
        // 위젯 숨기기
        TextBlock_Damage->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UDamageWidget::ShowWidget()
{
    if (TextBlock_Damage)
    {
        // 위젯 보이기
        TextBlock_Damage->SetVisibility(ESlateVisibility::Visible);
    }
}

void UDamageWidget::PlayNormalDamageAnimation(float Damage)
{
    if (damageAnimNormalDamage)
    {
        // 애니메이션 재생
        PlayAnimation(damageAnimNormalDamage);

        // 데미지 텍스트 설정
        TextBlock_Damage->SetText(FText::AsNumber(Damage));

        // 텍스트를 보이게 설정
        ShowWidget();
    }
}