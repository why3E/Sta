#include "PlayerWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "TimerManager.h"
#include "Enums.h"
#include "Engine/World.h"

void UPlayerWidget::NativeConstruct()
{
    Super::NativeConstruct();
    TextBlock_SkillQ->SetVisibility(ESlateVisibility::Hidden);
    TextBlock_SkillE->SetVisibility(ESlateVisibility::Hidden);

    if (Border_P2HP) Border_P2HP->SetRenderOpacity(0.f);
    if (Border_P2MP) Border_P2MP->SetRenderOpacity(0.f);
    if (Border_P3HP) Border_P3HP->SetRenderOpacity(0.f);
    if (Border_P3MP) Border_P3MP->SetRenderOpacity(0.f);
    if (Border_P4HP) Border_P4HP->SetRenderOpacity(0.f);
    if (Border_P4MP) Border_P4MP->SetRenderOpacity(0.f);

    if (MainPlayer2) MainPlayer2->SetRenderOpacity(0.f);
    if (MainPlayer3) MainPlayer3->SetRenderOpacity(0.f);
    if (MainPlayer4) MainPlayer4->SetRenderOpacity(0.f);

    if (Image_P2Back) Image_P2Back->SetRenderOpacity(0.f);
    if (Image_P3Back) Image_P3Back->SetRenderOpacity(0.f);
    if (Image_P4Back) Image_P4Back->SetRenderOpacity(0.f);

    if (Image_P2Hp) Image_P2Hp->SetRenderOpacity(0.f);
    if (Image_P3Hp) Image_P3Hp->SetRenderOpacity(0.f);
    if (Image_P4Hp) Image_P4Hp->SetRenderOpacity(0.f);

    if (Image_P2Mp) Image_P2Mp->SetRenderOpacity(0.f);
    if (Image_P3Mp) Image_P3Mp->SetRenderOpacity(0.f);
    if (Image_P4Mp) Image_P4Mp->SetRenderOpacity(0.f);

    if (Image_P2) Image_P2->SetRenderOpacity(0.f);
    if (Image_P3) Image_P3->SetRenderOpacity(0.f);
    if (Image_P4) Image_P4->SetRenderOpacity(0.f);
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

void UPlayerWidget::UpdateStBar(float CurrenSt, float MaxSt)
{
    if (ProgressBar_St)
    {
        ProgressBar_St->SetPercent(FMath::Clamp(CurrenSt / MaxSt, 0.0f, 1.0f));
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
    else if (TextBlock_SkillE && !IsQSkill)
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
    SetSkillIconInternal(Eskill, EAttack, ESkillType);
}

void UPlayerWidget::SetSkillIconInternal(UImage* Image, UImage* Image2, EClassType Type)
{
    if (!Image || !Image2) return;
    
    UTexture2D* SkillTexture = nullptr;
    UTexture2D* AttackTexture = nullptr;

    switch (Type)
    {
    case EClassType::CT_Fire:
        SkillTexture = FireSkillIcon;
        AttackTexture = FireAttackIcon;
        break;
    case EClassType::CT_Ice:
        SkillTexture = IceSkillIcon;
        AttackTexture = IceAttackIcon;
        break;
    case EClassType::CT_Wind:
        SkillTexture = WindSkillIcon;
        AttackTexture = WindAttackIcon;
        break;
    case EClassType::CT_Stone:
        SkillTexture = StoneSkillIcon;
        AttackTexture = StoneAttackIcon;
        break;
    default:
        break;
    }
    
    if (SkillTexture)
    {
        Image->SetBrushFromTexture(SkillTexture);
    }
    if (AttackTexture)
    {
        Image2->SetBrushFromTexture(AttackTexture);
    }
}

void UPlayerWidget::ShowPlayers(int32 PlayerCount)
{
	// Player 2
	if (PlayerCount >= 2)
	{
		if (Image_P2Back) Image_P2Back->SetRenderOpacity(0.5f);
		if (Image_P2)      Image_P2->SetRenderOpacity(1.f);
		if (Image_P2Hp)    Image_P2Hp->SetRenderOpacity(1.f);
		if (Image_P2Mp)    Image_P2Mp->SetRenderOpacity(1.f);
		if (ProgressBar_Hp_P2) ProgressBar_Hp_P2->SetRenderOpacity(1.f);
		if (ProgressBar_Mp_P2) ProgressBar_Mp_P2->SetRenderOpacity(1.f);
		if (Border_P2HP)   Border_P2HP->SetRenderOpacity(1.f);
		if (Border_P2MP)   Border_P2MP->SetRenderOpacity(1.f);
		if (MainPlayer2)   MainPlayer2->SetRenderOpacity(1.f);
	}

	// Player 3
	if (PlayerCount >= 3)
	{
		if (Image_P3Back) Image_P3Back->SetRenderOpacity(0.5f);
		if (Image_P3)      Image_P3->SetRenderOpacity(1.f);
		if (Image_P3Hp)    Image_P3Hp->SetRenderOpacity(1.f);
		if (Image_P3Mp)    Image_P3Mp->SetRenderOpacity(1.f);
		if (ProgressBar_Hp_P3) ProgressBar_Hp_P3->SetRenderOpacity(1.f);
		if (ProgressBar_Mp_P3) ProgressBar_Mp_P3->SetRenderOpacity(1.f);
		if (Border_P3HP)   Border_P3HP->SetRenderOpacity(1.f);
		if (Border_P3MP)   Border_P3MP->SetRenderOpacity(1.f);
		if (MainPlayer3)   MainPlayer3->SetRenderOpacity(1.f);
	}

	// Player 4
	if (PlayerCount >= 4)
	{
		if (Image_P4Back) Image_P4Back->SetRenderOpacity(0.5f);
		if (Image_P4)      Image_P4->SetRenderOpacity(1.f);
		if (Image_P4Hp)    Image_P4Hp->SetRenderOpacity(1.f);
		if (Image_P4Mp)    Image_P4Mp->SetRenderOpacity(1.f);
		if (ProgressBar_Hp_P4) ProgressBar_Hp_P4->SetRenderOpacity(1.f);
		if (ProgressBar_Mp_P4) ProgressBar_Mp_P4->SetRenderOpacity(1.f);
		if (Border_P4HP)   Border_P4HP->SetRenderOpacity(1.f);
		if (Border_P4MP)   Border_P4MP->SetRenderOpacity(1.f);
		if (MainPlayer4)   MainPlayer4->SetRenderOpacity(1.f);
	}
}

void UPlayerWidget::UpdateHpMpBar(float CurrentHp, float CurrentMp, int32 BarIndex, float MaxHp, float MaxMp)
{
    float HpPercent = FMath::Clamp(CurrentHp / MaxHp, 0.0f, 1.0f);
    float MpPercent = FMath::Clamp(CurrentMp / MaxMp, 0.0f, 1.0f);

    switch (BarIndex)
    {
    case 2:
        if (ProgressBar_Hp_P2)
            ProgressBar_Hp_P2->SetPercent(HpPercent);
        if (ProgressBar_Mp_P2)
            ProgressBar_Mp_P2->SetPercent(MpPercent);
        break;

    case 3:
        if (ProgressBar_Hp_P3)
            ProgressBar_Hp_P3->SetPercent(HpPercent);
        if (ProgressBar_Mp_P3)
            ProgressBar_Mp_P3->SetPercent(MpPercent);
        break;

    case 4:
        if (ProgressBar_Hp_P4)
            ProgressBar_Hp_P4->SetPercent(HpPercent);
        if (ProgressBar_Mp_P4)
            ProgressBar_Mp_P4->SetPercent(MpPercent);
        break;

    default:
        break;
    }
}

void UPlayerWidget::UpdatePotionIcons(const FItemInventory& Inventory)
{
    auto SetPotionUI = [](UImage* Image, UTextBlock* CountText, int32 LargeCount, int32 SmallCount, UTexture2D* LargeIcon, UTexture2D* SmallIcon)
    {
        if (LargeCount > 0)
        {
            if (Image) Image->SetBrushFromTexture(LargeIcon);
            if (CountText) CountText->SetText(FText::AsNumber(LargeCount));
        }
        else if (SmallCount > 0)
        {
            if (Image) Image->SetBrushFromTexture(SmallIcon);
            if (CountText) CountText->SetText(FText::AsNumber(SmallCount));
        }
        else
        {
            if (Image) Image->SetBrushFromTexture(nullptr);
            if (CountText) CountText->SetText(FText::FromString(TEXT("0")));
        }
    };

    SetPotionUI(
        HPPosion,
        HPPosion_count,
        Inventory.GetCount(EItemDropType::HealPotion_L),
        Inventory.GetCount(EItemDropType::HealPotion_S),
        HPPosionLIcon,
        HPPosionSIcon
    );

    SetPotionUI(
        MPPosion,
        MPPosion_count,
        Inventory.GetCount(EItemDropType::ManaPotion_L),
        Inventory.GetCount(EItemDropType::ManaPotion_S),
        MPPosionLIcon,
        MPPosionSIcon
    );

    SetPotionUI(
        STPosion,
        STPosion_count,
        Inventory.GetCount(EItemDropType::StaminaPotion_L),
        Inventory.GetCount(EItemDropType::StaminaPotion_S),
        STPosionLIcon,
        STPosionSIcon
    );
}