#include "PlayerWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"
#include "Enums.h"
#include "Engine/World.h"

void UPlayerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    TextBlock_SkillQ->SetVisibility(ESlateVisibility::Hidden);
    TextBlock_SkillE->SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerWidget::UpdateHpBar(float CurrentHp, float MaxHp)
{
    if (ProgressBar_Hp)
    {
        ProgressBar_Hp->SetPercent(FMath::Clamp(CurrentHp / MaxHp, 0.0f, 1.0f));
    }
}

void UPlayerWidget::UpdateMpBar(float CurrentMp, float MaxMp)
{
    if (ProgressBar_Mp)
    {
        ProgressBar_Mp->SetPercent(FMath::Clamp(CurrentMp / MaxMp, 0.0f, 1.0f));
    }
}

void UPlayerWidget::UpdateCoolTimeText()
{
    if (TextBlock_SkillQ && IsQSkill)
    {
        TextBlock_SkillQ->SetVisibility(ESlateVisibility::Visible);

        if (CurrentQCoolTime > 0.0f)
        {
            CurrentQCoolTime -= 0.1f;

            // 소수점 1자리로 출력
            if (CurrentQCoolTime < 0.9f)
            {
                TextBlock_SkillQ->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), CurrentQCoolTime)));
            }
            else
            {
                // 정수로 출력
                TextBlock_SkillQ->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentQCoolTime))));
            }
        }

        // 쿨타임 종료 처리
        if (CurrentQCoolTime <= 0.0f)
        {
            GetWorld()->GetTimerManager().ClearTimer(th_skillQCoolTime);
            TextBlock_SkillQ->SetVisibility(ESlateVisibility::Hidden);
        }
    }
    else if(TextBlock_SkillE && !IsQSkill)
    {
        TextBlock_SkillE->SetVisibility(ESlateVisibility::Visible);

        if (CurrentECoolTime > 0.0f)
        {
            CurrentECoolTime -= 0.1f;

            // 소수점 1자리로 출력
            if (CurrentECoolTime < 0.9f)
            {
                TextBlock_SkillE->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), CurrentECoolTime)));
            }
            else
            {
                // 정수로 출력
                TextBlock_SkillE->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::FloorToInt(CurrentECoolTime))));
            }
        }

        // 쿨타임 종료 처리
        if (CurrentECoolTime <= 0.0f)
        {
            GetWorld()->GetTimerManager().ClearTimer(th_skillECoolTime);
            TextBlock_SkillE->SetVisibility(ESlateVisibility::Hidden);
        }
    }
}

void UPlayerWidget::UpdateCountDown(float CoolTime, bool bIsQSkill)
{
    IsQSkill = bIsQSkill;

    if (bIsQSkill){
        CurrentQCoolTime = CoolTime;

        if (CurrentQCoolTime > 1.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(th_skillQCoolTime, this, &UPlayerWidget::UpdateCoolTimeText, 0.1f, true);
        }
        else if (CurrentQCoolTime <= 0.0f)
        {
            GetWorld()->GetTimerManager().ClearTimer(th_skillQCoolTime);
            CurrentQCoolTime = 0.0f;
        }
        if(SkillQAnim)
        {
            PlayAnimation(SkillQAnim);
        }
    }
    else
    {
        CurrentECoolTime = CoolTime;

        if (CurrentECoolTime > 1.0f)
        {
            GetWorld()->GetTimerManager().SetTimer(th_skillECoolTime, this, &UPlayerWidget::UpdateCoolTimeText, 0.1f, true);
        }
        else if (CurrentECoolTime <= 0.0f)
        {
            GetWorld()->GetTimerManager().ClearTimer(th_skillECoolTime);
            CurrentECoolTime = 0.0f;
        }
        if(SkillEAnim)
        {
            PlayAnimation(SkillEAnim);
        }
    }
}

void UPlayerWidget::SetQSkillIcon(EClassType QSkillType)
{
    SetSkillIconInternal(Qskill, QAttack, QSkillType);
}

void UPlayerWidget::SetESkillIcon(EClassType ESkillType)
{
    SetSkillIconInternal(Eskill,EAttack, ESkillType);
}

void UPlayerWidget::SetSkillIconInternal(UImage* Image, UImage* Image2, EClassType Type)
{
    if (!Image) return;
    if (!Image2) return;
    
    FString Path;
    FString Path2;

    switch (Type)
    {
    case EClassType::CT_Fire:
        Path = TEXT("/Game/HUD/Fire_Skill.Fire_Skill");
        Path2 = TEXT("/Game/HUD/Fire_Attack.Fire_Attack");
        break;
    case EClassType::CT_Ice:
        Path = TEXT("/Game/HUD/Ice_Skill.Ice_Skill");
        Path2 = TEXT("/Game/HUD/Ice_Attack.Ice_Attack");
        break;
    case EClassType::CT_Wind:
        Path = TEXT("/Game/HUD/Wind_Skill.Wind_Skill");
        Path2 = TEXT("/Game/HUD/Wind_Attack.Wind_Attack");
        break;
    case EClassType::CT_Stone:
        Path = TEXT("/Game/HUD/Stone_Skill.Stone_Skill");
        Path2 = TEXT("/Game/HUD/Stone_Attack.Stone_Attack");
        break;
    default:
        Path = TEXT("");
        Path2 = TEXT("");
        break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Setting skill icon from path: %s"), *Path);
    if (!Path.IsEmpty())
    {
        UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *Path);
        if (Texture)
        {
            Image->SetBrushFromTexture(Texture);
        }
    }
    if (!Path2.IsEmpty())
    {
        UTexture2D* Texture2 = LoadObject<UTexture2D>(nullptr, *Path2);
        if (Texture2)
        {
            Image2->SetBrushFromTexture(Texture2);
        }
    }
}
