// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyFadeWidget.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API UMyFadeWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	UPROPERTY(meta = (BindWidget))
	class UBorder* Border_Fade;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* FadeIn;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	class UWidgetAnimation* FadeOut;

	UFUNCTION()
	void PlayFadeIn();

	UFUNCTION()
	void PlayFadeOut();
};
