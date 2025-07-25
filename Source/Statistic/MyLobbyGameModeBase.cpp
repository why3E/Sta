#include "MyLobbyGameModeBase.h"
#include "MyLobbyWidget.h"
#include "MyLobbyReadyWidget.h"
#include "Blueprint/UserWidget.h"

void AMyLobbyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (LobbyUIClass)
	{
		LobbyUI = CreateWidget<UMyLobbyWidget>(GetWorld(), LobbyUIClass);
		if (LobbyUI)
		{
			LobbyUI->AddToViewport();

			// 마우스 커서 보이게 설정
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (PC)
			{
				PC->bShowMouseCursor = true;
				PC->SetInputMode(FInputModeUIOnly());
			}
            LobbyUI->OnStartPressed.AddDynamic(this, &AMyLobbyGameModeBase::HandleStartPressed);
		}
	}
}

void AMyLobbyGameModeBase::HandleStartPressed(const FString& IP)
{
    EnteredIP = IP;
    UE_LOG(LogTemp, Warning, TEXT("GameMode에서 받은 IP: %s"), *EnteredIP);

    // 기존 LobbyUI 제거
    if (LobbyUI)
    {
        LobbyUI->RemoveFromParent();
        LobbyUI = nullptr;
    }

    // LobbyReadyUI 생성 및 뷰포트에 추가
    if (LobbyReadyUIClass)
    {
        LobbyReadyUI = CreateWidget<UMyLobbyReadyWidget>(GetWorld(), LobbyReadyUIClass);
        if (LobbyReadyUI)
        {
            LobbyReadyUI->AddToViewport();

            // 마우스 커서 보이기 및 UI 입력 모드
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                PC->bShowMouseCursor = true;
                PC->SetInputMode(FInputModeUIOnly());
            }

            // 나가기 버튼 이벤트 바인딩
            LobbyReadyUI->OnOutPressed.AddDynamic(this, &AMyLobbyGameModeBase::HandleOutPressed);
        }
    }
}

void AMyLobbyGameModeBase::HandleOutPressed()
{
    // LobbyReadyUI 제거
    if (LobbyReadyUI)
    {
        LobbyReadyUI->RemoveFromParent();
        LobbyReadyUI = nullptr;
    }

    // LobbyUI 재생성 및 뷰포트 추가
    if (LobbyUIClass)
    {
        LobbyUI = CreateWidget<UMyLobbyWidget>(GetWorld(), LobbyUIClass);
        if (LobbyUI)
        {
            LobbyUI->AddToViewport();

            // 마우스 커서 보이게 및 UI 입력 모드
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                PC->bShowMouseCursor = true;
                PC->SetInputMode(FInputModeUIOnly());
            }

            // 다시 시작 버튼 이벤트 바인딩
            LobbyUI->OnStartPressed.AddDynamic(this, &AMyLobbyGameModeBase::HandleStartPressed);
        }
    }
}