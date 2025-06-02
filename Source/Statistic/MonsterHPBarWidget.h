// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "MonsterHPBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API UMonsterHPBarWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* pb_healthBar;

	UFUNCTION()
	void updateHpBar(float currentHp, float maxHp);
};
