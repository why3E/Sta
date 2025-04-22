// Fill out your copyright notice in the Description page of Project Settings.


#include "MMComboActionData.h"

UMMComboActionData::UMMComboActionData()
{   
    // 기본값 설정
    SectionPrefix = TEXT("BasicComboAttack");
    FrameRate = 60.0f;
    MaxComboCount = 3;
    ComboFrame.Add(30.0f); // 첫 번째 콤보의 입력 프레임
    ComboFrame.Add(30.0f); // 두 번째 콤보의 입력 프레임
    ComboFrame.Add(-1.0f); // 세 번째 콤보의 입력 프레임
}