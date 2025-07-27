// Fill out your copyright notice in the Description page of Project Settings.

#include "SESSION.h"
#include "MyWorldMapWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "MyMagicStatue.h"
#include "PlayerCharacter.h"
#include "EngineUtils.h"
#include "Components/Button.h"

void UMyWorldMapWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 이 위젯이 포커스 받을 수 있도록 설정
	SetIsFocusable(true);
    
    if (Button_0) Button_0->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OnButton0Clicked);
    if (Button_1) Button_1->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OnButton1Clicked);
    if (Button_2) Button_2->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OnButton2Clicked);
    if (Button_3) Button_3->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OnButton3Clicked);
    if (Button_4) Button_4->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OnButton4Clicked);
    if (Button_5) Button_5->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OnButton5Clicked);
    if (Button_6) Button_6->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OnButton6Clicked);
    if (Button_7) Button_7->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OnButton7Clicked);

    if (OutButton)
    {
        OutButton->OnClicked.AddDynamic(this, &UMyWorldMapWidget::OutButtonClicked);
    }

    if (Number1)
	{
		// 초기 위치 조정 (선택사항)
		Number1->SetRenderTranslation(FVector2D::ZeroVector);
	}

    if (UWorld* World = GetWorld())
    {
        InitializeStatues(World);
    }

    for (int i = 0; i < 4; ++i) {
        if (nullptr == g_c_players[i]) {
            switch (i) {
            case 0:
                Number1->SetVisibility(ESlateVisibility::Hidden);
                break;

            case 1:
                Number2->SetVisibility(ESlateVisibility::Hidden);
                break;

            case 2:
                Number3->SetVisibility(ESlateVisibility::Hidden);
                break;

            case 3:
                Number4->SetVisibility(ESlateVisibility::Hidden);
                break;
            }
        }
    }
}

void UMyWorldMapWidget::InitializeStatues(UWorld* World)
{
    Statues.Empty();

    for (TActorIterator<AMyMagicStatue> It(World); It; ++It)
    {
        Statues.Add(*It);
    }

    UE_LOG(LogTemp, Warning, TEXT("Found %d statues"), Statues.Num());
}

void UMyWorldMapWidget::TeleportToStatueByIndex(int32 Index)
{
    if (!Statues.IsValidIndex(Index)) return;
    UE_LOG(LogTemp, Error, TEXT("Index : %d"), Index);

    AMyMagicStatue* Statue = Statues[Index];
    if (!Statue) return;

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APlayerCharacter* Player = Cast<APlayerCharacter>(PC->GetPawn());
    if (!Player) return;

    Statue->Interact(Player);
}

void UMyWorldMapWidget::OnButton0Clicked() { TeleportToStatueByIndex(0); }
void UMyWorldMapWidget::OnButton1Clicked() { TeleportToStatueByIndex(1); }
void UMyWorldMapWidget::OnButton2Clicked() { TeleportToStatueByIndex(2); }
void UMyWorldMapWidget::OnButton3Clicked() { TeleportToStatueByIndex(3); }
void UMyWorldMapWidget::OnButton4Clicked() { TeleportToStatueByIndex(4); }
void UMyWorldMapWidget::OnButton5Clicked() { TeleportToStatueByIndex(5); }
void UMyWorldMapWidget::OnButton6Clicked() { TeleportToStatueByIndex(6); }
void UMyWorldMapWidget::OnButton7Clicked() { TeleportToStatueByIndex(7); }

void UMyWorldMapWidget::UpdateNumberPosition(char index, const FVector2D& WorldPos)
{
	// X,Y 축이 반전됨에 주의
	float XRatio = (50400.f - WorldPos.X) / 100800.f;
	float YRatio = (WorldPos.Y + 50400.f) / 100800.f;

	float ImageX = (XRatio - 0.5f) * 660.f;
	float ImageY = (YRatio - 0.5f) * 690.f;

    switch (index) {
    case 0:
        if (!Number1) return;

        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Number1->Slot)) {
            CanvasSlot->SetPosition(FVector2D(ImageY, ImageX));
        }
        break;

    case 1:
        if (!Number2) return;

        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Number2->Slot)) {
            CanvasSlot->SetPosition(FVector2D(ImageY, ImageX));
        }
        break;

    case 2:
        if (!Number3) return;

        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Number3->Slot)) {
            CanvasSlot->SetPosition(FVector2D(ImageY, ImageX));
        }
        break;

    case 3:
        if (!Number4) return;

        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Number4->Slot)) {
            CanvasSlot->SetPosition(FVector2D(ImageY, ImageX));
        }
        break;
    }
}

void UMyWorldMapWidget::OutButtonClicked()
{
    UE_LOG(LogTemp, Warning, TEXT("Out button clicked"));
    
    // 델리게이트 호출
    OnMapCloseRequested.Broadcast();
}