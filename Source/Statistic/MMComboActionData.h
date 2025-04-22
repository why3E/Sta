// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MMComboActionData.generated.h"

UCLASS()
class STATISTIC_API UMMComboActionData : public UDataAsset
{
	GENERATED_BODY()
public:
	UMMComboActionData();

public:
	// 몽타주 섹션 이름 (접두사)
	UPROPERTY(EditAnywhere, Category = Name)
	FString SectionPrefix;

	// 재생 속도
	UPROPERTY(EditAnywhere, Category = ComboData)
	float FrameRate;

	// 최대 가능 콤보 수
	UPROPERTY(EditAnywhere, Category = ComboData)
	uint8 MaxComboCount;

	// 콤보별 다음 콤보로 넘어가기 위한 입력 프레임 정보
	UPROPERTY(EditAnywhere, Category = ComboData)
	TArray<float> ComboFrame;
};
