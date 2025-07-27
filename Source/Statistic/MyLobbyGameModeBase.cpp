#include "SESSION.h"
#include "MyLobbyGameModeBase.h"
#include "MyLobbyWidget.h"
#include "MyLobbyReadyWidget.h"
#include "Blueprint/UserWidget.h"

constexpr short SERVER_PORT = 5000;

void AMyLobbyGameModeBase::BeginPlay()
{
	Super::BeginPlay();

    MyLobbyGameModeBase = this;

    WSADATA WSAData;
    auto ret = WSAStartup(MAKEWORD(2, 2), &WSAData);

    g_l_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    for (int i = 0; i < 8; ++i) {
        g_elements[i] = (0 == i % 2) ? 0 : 1;
    }

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

    GetWorld()->GetTimerManager().SetTimer(SleepExTimerHandle, this, &AMyLobbyGameModeBase::SleepExTimer, 1.0f, true);
}

void AMyLobbyGameModeBase::HandleStartPressed(const FString& IP)
{
    EnteredIP = IP;
    UE_LOG(LogTemp, Warning, TEXT("GameMode에서 받은 IP: %s"), *EnteredIP);

    const char* ipCStr = TCHAR_TO_UTF8(*EnteredIP);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, ipCStr, &addr.sin_addr);

    auto ret = WSAConnect(g_l_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);

    DWORD recv_bytes;
    DWORD recv_flag = 0;
    ret = WSARecv(g_l_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, g_recv_callback);

    cs_init_lobby_packet p;
    p.packet_size = sizeof(cs_init_lobby_packet);
    p.packet_type = C2S_INIT_LOBBY_PACKET;
    strncpy(p.ip_address, ipCStr, sizeof(p.ip_address) - 1);
    g_do_send(&p);
}

void AMyLobbyGameModeBase::SleepExTimer() {
    if (!g_is_running) {
        SleepEx(1, TRUE);
        return;
    }

    GetWorld()->GetTimerManager().ClearTimer(SleepExTimerHandle);
}

void AMyLobbyGameModeBase::HandleOutPressed()
{
    cs_remove_player_packet p;
    p.packet_size = sizeof(cs_remove_player_packet);
    p.packet_type = C2S_REMOVE_PLAYER_PAKCET;
    g_do_send(&p);

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