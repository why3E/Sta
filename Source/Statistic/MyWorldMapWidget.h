// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyWorldMapWidget.generated.h"

class UImage;
class UButton;
class AMyMagicStatue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMapCloseRequested);

UCLASS()
class STATISTIC_API UMyWorldMapWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 이미지 변수
	UPROPERTY(meta = (BindWidget))
	UImage* Number1;

	UPROPERTY(meta = (BindWidget))
	UImage* Number2;

	UPROPERTY(meta = (BindWidget))
	UImage* Number3;

	UPROPERTY(meta = (BindWidget))
	UImage* Number4;

	// 버튼 변수
	UPROPERTY(meta = (BindWidget))
	UButton* Button_0;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_1;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_2;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_3;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_4;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_5;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_6;

	UPROPERTY(meta = (BindWidget))
	UButton* Button_7;

	UPROPERTY(meta = (BindWidget))
	UButton* OutButton;

public:
    // 델리게이트 인스턴스
    UPROPERTY(BlueprintAssignable, Category="Map")
    FOnMapCloseRequested OnMapCloseRequested;

    UFUNCTION()
    void OutButtonClicked();

	UFUNCTION()
    void OnButton0Clicked();

    UFUNCTION()
    void OnButton1Clicked();

    UFUNCTION()
    void OnButton2Clicked();

    UFUNCTION()
    void OnButton3Clicked();

    UFUNCTION()
    void OnButton4Clicked();

    UFUNCTION()
    void OnButton5Clicked();

    UFUNCTION()
    void OnButton6Clicked();

    UFUNCTION()
    void OnButton7Clicked();

	// 월드 좌표를 받아 이미지 위치로 변환 후 아이콘 이동
	void UpdateNumber1Position(const FVector2D& WorldPos);

	void InitializeStatues(UWorld* World);

private:
	// 월드 영역(최소/최대)
	FVector2D WorldMin = FVector2D(-50400.0f, -50400.0f);
	FVector2D WorldMax = FVector2D(50400.0f, 50400.0f);
	FVector2D WorldSize = FVector2D(100800.0f, 100800.0f);

	// 이미지 크기(px) - 필요하면 UMG에서 동적으로 얻어도 됨
	FVector2D ImageSize = FVector2D(800.0f, 800.0f);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Statues")
    TArray<AMyMagicStatue*> Statues;
public:
    UFUNCTION()
    void TeleportToStatueByIndex(int32 Index);

};
