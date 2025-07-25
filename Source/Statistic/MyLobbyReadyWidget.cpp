// Fill out your copyright notice in the Description page of Project Settings.


#include "MyLobbyReadyWidget.h"
#include "Enums.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UMyLobbyReadyWidget::NativeConstruct()
{
	Super::NativeConstruct();

    // 초기 속성 설정
    CurrentClassTypeLeft = EClassType::CT_Wind;
    CurrentClassTypeRight = EClassType::CT_Fire;

    // 이미지 초기화
    UpdateImageByClassType(CurrentClassTypeLeft, MainLeft);
    UpdateImageByClassType(CurrentClassTypeRight, MainRight);

    if (Number1OnOff)
        Number1OnOff->SetOpacity(1.0f);

    if (Number2OnOff)
        Number2OnOff->SetOpacity(0.25f);

    if (Number3OnOff)
        Number3OnOff->SetOpacity(0.25f);

    if (Number4OnOff)
        Number4OnOff->SetOpacity(0.25f);

    // 텍스트 초기화
    if (Number1)
        Number1->SetText(FText::FromString(TEXT("Player 1")));

    if (Number2)
        Number2->SetText(FText::FromString(TEXT("Open")));

    if (Number3)
        Number3->SetText(FText::FromString(TEXT("Open")));

    if (Number4)
        Number4->SetText(FText::FromString(TEXT("Open")));

	if (StartButton)
	{
		StartButton->OnClicked.AddUniqueDynamic(this, &UMyLobbyReadyWidget::OnStartButtonClicked);
	}

	if (OutButton)
	{
		OutButton->OnClicked.AddUniqueDynamic(this, &UMyLobbyReadyWidget::OnOutButtonClicked);
	}

	if (ChangeLeft)
	{
		ChangeLeft->OnClicked.AddUniqueDynamic(this, &UMyLobbyReadyWidget::OnChangeLeftClicked);
	}

	if (ChangeRight)
	{
		ChangeRight->OnClicked.AddUniqueDynamic(this, &UMyLobbyReadyWidget::OnChangeRightClicked);
	}
}

void UMyLobbyReadyWidget::OnStartButtonClicked()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC)
    {
        // 마우스 커서 숨기기
        PC->bShowMouseCursor = false;

        // 입력 모드를 게임 플레이 모드로 전환 (게임 및 UI 입력 모두 허용)
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
    }
    // 월드 컨텍스트 가져오기
    UWorld* World = GetWorld();
    if (World)
    {
        // 레벨 경로는 프로젝트 내 경로 (패키지 경로)
        FString LevelName = TEXT("DemoLevel1");

        UGameplayStatics::OpenLevel(World, FName(*LevelName));
    }
}

void UMyLobbyReadyWidget::OnOutButtonClicked()
{
    // 델리게이트 호출 (게임모드에서 바인딩할 것)
    OnOutPressed.Broadcast();
}

void UMyLobbyReadyWidget::OnChangeLeftClicked()
{
    switch (CurrentClassTypeLeft)
    {
    case EClassType::CT_Wind:  CurrentClassTypeLeft = EClassType::CT_Stone; break;
    case EClassType::CT_Stone: CurrentClassTypeLeft = EClassType::CT_Ice;   break;
    case EClassType::CT_Ice:   CurrentClassTypeLeft = EClassType::CT_Fire;  break;
    case EClassType::CT_Fire:  CurrentClassTypeLeft = EClassType::CT_Wind;  break;
    default:                   CurrentClassTypeLeft = EClassType::CT_Wind;  break;
    }

    UpdateImageByClassType(CurrentClassTypeLeft, MainLeft);
}

void UMyLobbyReadyWidget::OnChangeRightClicked()
{
    switch (CurrentClassTypeRight)
    {
    case EClassType::CT_Wind:  CurrentClassTypeRight = EClassType::CT_Fire;  break;
    case EClassType::CT_Fire:  CurrentClassTypeRight = EClassType::CT_Ice;   break;
    case EClassType::CT_Ice:   CurrentClassTypeRight = EClassType::CT_Stone; break;
    case EClassType::CT_Stone: CurrentClassTypeRight = EClassType::CT_Wind;  break;
    default:                   CurrentClassTypeRight = EClassType::CT_Wind;  break;
    }

    UpdateImageByClassType(CurrentClassTypeRight, MainRight);
}

void UMyLobbyReadyWidget::UpdateImageByClassType(EClassType ClassType, UImage* ImageWidget)
{
    if (!ImageWidget) return;

    UTexture2D* NewTexture = nullptr;
    switch (ClassType)
    {
    case EClassType::CT_Wind:
        NewTexture = AttributeWindTexture.Get();
        break;
    case EClassType::CT_Fire:
        NewTexture = AttributeFireTexture.Get();
        break;
    case EClassType::CT_Ice:
        NewTexture = AttributeIceTexture.Get();
        break;
    case EClassType::CT_Stone:
        NewTexture = AttributeStoneTexture.Get();
        break;
    default:
        break;
    }

    if (NewTexture)
    {
        FSlateBrush NewBrush;
        NewBrush.SetResourceObject(NewTexture);
        NewBrush.ImageSize = FVector2D(NewTexture->GetSizeX(), NewTexture->GetSizeY());
        ImageWidget->SetBrush(NewBrush);
    }
}

