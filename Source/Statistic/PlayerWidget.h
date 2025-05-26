// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWidget.generated.h"

/**
 * 
 */
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
};
