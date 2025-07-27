// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Enums.h"
#include "PlayerWidget.generated.h"

class UProgressBar;
class UTextBlock;
class UBorder;

UCLASS()
class STATISTIC_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Hp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Mp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_St;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Cross;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_SkillQ;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock_SkillE;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* SkillQAnim;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* SkillEAnim;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Qskill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Eskill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> QAttack;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> EAttack;

	void UpdateHpBar(float CurrentHp, float MaxHp);
	void UpdateMpBar(float CurrentMp, float MaxMp);
	void UpdateStBar(float CurrenSt, float MaxSt);
	
	void UpdateCountDown(float CoolTime, bool bIsQSkill);
	void UpdateCoolTimeText();

private:
	float CurrentQCoolTime = 0.0f;
	FTimerHandle th_skillQCoolTime; // 타이머 핸들

	float CurrentECoolTime = 0.0f;
	FTimerHandle th_skillECoolTime; // 타이머 핸들

	bool IsQSkill = false;

public:
	UFUNCTION(BlueprintCallable)
	void SetQSkillIcon(EClassType QSkillType);

	UFUNCTION(BlueprintCallable)
	void SetESkillIcon(EClassType ESkillType);

	UFUNCTION(BlueprintCallable)
	void SetSkillIconInternal(UImage* Image,UImage* Image2, EClassType Type);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* FireSkillIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* FireAttackIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* IceSkillIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* IceAttackIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* WindSkillIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* WindAttackIcon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* StoneSkillIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* StoneAttackIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* HPPosionLIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* HPPosionSIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* MPPosionLIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* MPPosionSIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* STPosionLIcon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
    UTexture2D* STPosionSIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HPPosion;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MPPosion;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> STPosion;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> HPPosion_count;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MPPosion_count;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> STPosion_count;


	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Hp_P2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Mp_P2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Hp_P3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Mp_P3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Hp_P4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> ProgressBar_Mp_P4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P2Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P3Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P4Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P4;

	// 플레이어 텍스트
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MainPlayer1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MainPlayer2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MainPlayer3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MainPlayer4;

	// MP 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P4Mp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P3Mp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P2Mp;

	// HP 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P4Hp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P3Hp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_P2Hp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_P2HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_P2MP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_P3HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_P3MP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_P4HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> Border_P4MP;
public:
	UFUNCTION()
	void ShowPlayers(int32 PlayerCount);
	UFUNCTION()
	void UpdateHpMpBar(float CurrentHp, float CurrentMp, int32 BarIndex, float MaxHp = 100.0f, float MaxMp = 100.0f);

	void UpdatePotionIcons(const FItemInventory& Inventory);
};
