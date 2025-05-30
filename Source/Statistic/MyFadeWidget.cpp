// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFadeWidget.h"

void UMyFadeWidget::PlayFadeIn()
{
	PlayAnimation(FadeIn);
}

void UMyFadeWidget::PlayFadeOut()
{
	PlayAnimation(FadeOut);
}