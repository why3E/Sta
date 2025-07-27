#pragma once

#include "SESSION.h"
#include "MyLobbyWidget.h"
#include "MyPlayerController.h"
#include "MyLobbyGameModeBase.h"
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Enums.h"
#include "Kismet/GameplayStatics.h"
#include "MyLobbyReadyWidget.generated.h"

extern EXP_OVER g_recv_over;
extern int g_remained;

inline void g_do_send(void* buff);
inline void g_process_packet(char* packet);
inline void CALLBACK g_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
inline void CALLBACK g_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOutPressed);

UCLASS()
class STATISTIC_API UMyLobbyReadyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 이미지: Number1OnOff ~ Number4OnOff
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number1OnOff;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number2OnOff;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number3OnOff;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number4OnOff;

	// 버튼: StartButton, OutButton
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> StartButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> OutButton;

	// 텍스트 블록: Number1 ~ Number4
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Number1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Number2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Number3;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Number4;

	// 이미지: Number1Left ~ Number4Left, Number1Right ~ Number4Right
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number1Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number1Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number2Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number2Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number3Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number3Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number4Left;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Number4Right;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MainLeft;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> MainRight;

	// 버튼: ChangeLeft, ChangeRight
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChangeLeft;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChangeRight;

	// 텍스처: 사람 그림 및 속성 그림 (불, 얼음, 바람, 땅)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> CharacterTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> AttributeFireTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> AttributeIceTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> AttributeWindTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SkillIcon")
	TObjectPtr<UTexture2D> AttributeStoneTexture;

	// 버튼 바인딩용 함수들
	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnOutButtonClicked();

	UFUNCTION()
	void OnChangeLeftClicked();

	UFUNCTION()
	void OnChangeRightClicked();

	virtual void NativeConstruct() override;

	// 현재 선택된 속성
	EClassType CurrentClassTypeLeft = EClassType::CT_Wind;
	EClassType CurrentClassTypeRight = EClassType::CT_Wind;

	void UpdateImageByClassType(EClassType ClassType, UImage* ImageWidget);

public:
    UPROPERTY(BlueprintAssignable, Category="Events")
    FOnOutPressed OnOutPressed;
};

inline void g_do_send(void* buff) {
    EXP_OVER* o = new EXP_OVER;
    unsigned char packet_size = reinterpret_cast<unsigned char*>(buff)[0];
    memcpy(o->m_buffer, buff, packet_size);
    o->m_wsabuf[0].len = packet_size;

    DWORD send_bytes;
    auto ret = WSASend(g_l_socket, o->m_wsabuf, 1, &send_bytes, 0, &(o->m_over), g_send_callback);
    if (ret == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            delete o;
            return;
        }
    }
}

inline void g_process_packet(char* packet) {
    char packet_type = packet[1];
    UE_LOG(LogTemp, Warning, TEXT("Received Packet %d"), packet_type);

    switch (packet_type) {
    case S2C_ERROR_CODE_PACKET: {
        sc_error_code_packet* p = reinterpret_cast<sc_error_code_packet*>(packet);
        switch (p->ec) {
        case INIT:
            g_is_host = true;

            // 기존 LobbyUI 제거
            if (MyLobbyGameModeBase->LobbyUI) {
                MyLobbyGameModeBase->LobbyUI->RemoveFromParent();
                MyLobbyGameModeBase->LobbyUI = nullptr;
            }

            // LobbyReadyUI 생성 및 뷰포트에 추가
            if (MyLobbyGameModeBase->LobbyReadyUIClass) {
                MyLobbyGameModeBase->LobbyReadyUI = CreateWidget<UMyLobbyReadyWidget>(MyLobbyGameModeBase->GetWorld(), MyLobbyGameModeBase->LobbyReadyUIClass);

                if (MyLobbyGameModeBase->LobbyReadyUI) {
                    MyLobbyGameModeBase->LobbyReadyUI->AddToViewport();

                    // 마우스 커서 보이기 및 UI 입력 모드
                    APlayerController* PC = MyLobbyGameModeBase->GetWorld()->GetFirstPlayerController();

                    if (PC) {
                        PC->bShowMouseCursor = true;
                        PC->SetInputMode(FInputModeUIOnly());
                    }

                    // 나가기 버튼 이벤트 바인딩
                    MyLobbyGameModeBase->LobbyReadyUI->OnOutPressed.AddDynamic(MyLobbyGameModeBase, &AMyLobbyGameModeBase::HandleOutPressed);
                }
            }
            break;

        case SUCCEED:
            g_is_host = false;

            // 기존 LobbyUI 제거
            if (MyLobbyGameModeBase->LobbyUI) {
                MyLobbyGameModeBase->LobbyUI->RemoveFromParent();
                MyLobbyGameModeBase->LobbyUI = nullptr;
            }

            // LobbyReadyUI 생성 및 뷰포트에 추가
            if (MyLobbyGameModeBase->LobbyReadyUIClass) {
                MyLobbyGameModeBase->LobbyReadyUI = CreateWidget<UMyLobbyReadyWidget>(MyLobbyGameModeBase->GetWorld(), MyLobbyGameModeBase->LobbyReadyUIClass);

                if (MyLobbyGameModeBase->LobbyReadyUI) {
                    MyLobbyGameModeBase->LobbyReadyUI->AddToViewport();

                    // 마우스 커서 보이기 및 UI 입력 모드
                    APlayerController* PC = MyLobbyGameModeBase->GetWorld()->GetFirstPlayerController();

                    if (PC) {
                        PC->bShowMouseCursor = true;
                        PC->SetInputMode(FInputModeUIOnly());
                    }

                    // 나가기 버튼 이벤트 바인딩
                    MyLobbyGameModeBase->LobbyReadyUI->OnOutPressed.AddDynamic(MyLobbyGameModeBase, &AMyLobbyGameModeBase::HandleOutPressed);
                }
            }
            break;

        case FAILED:
            break;
        }
        break;
    }

    case S2C_INIT_LOBBY_PAKCET: {
        sc_init_lobby_packet* p = reinterpret_cast<sc_init_lobby_packet*>(packet);
        UE_LOG(LogTemp, Warning, TEXT("Left : %d. Right : %d"), p->player.left_element, p->player.right_element);
        switch (p->player.slot) {
        case 0:
            MyLobbyReadyWidget->Number1OnOff->SetOpacity(1.0f);
            MyLobbyReadyWidget->Number1->SetText(FText::FromString(TEXT("Player 1")));
            MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->player.left_element), MyLobbyReadyWidget->Number1Left);
            MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->player.right_element), MyLobbyReadyWidget->Number1Right);
            break;

        case 1:
            MyLobbyReadyWidget->Number2OnOff->SetOpacity(1.0f);
            MyLobbyReadyWidget->Number2->SetText(FText::FromString(TEXT("Player 2")));
            MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->player.left_element), MyLobbyReadyWidget->Number2Left);
            MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->player.right_element), MyLobbyReadyWidget->Number2Right);
            break;

        case 2:
            MyLobbyReadyWidget->Number3OnOff->SetOpacity(1.0f);
            MyLobbyReadyWidget->Number3->SetText(FText::FromString(TEXT("Player 3")));
            MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->player.left_element), MyLobbyReadyWidget->Number3Left);
            MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->player.right_element), MyLobbyReadyWidget->Number3Right);
            break;

        case 3:
            MyLobbyReadyWidget->Number4OnOff->SetOpacity(1.0f);
            MyLobbyReadyWidget->Number4->SetText(FText::FromString(TEXT("Player 4")));
            MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->player.left_element), MyLobbyReadyWidget->Number4Left);
            MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->player.right_element), MyLobbyReadyWidget->Number4Right);
            break;
        }
        break;
    }

    case S2C_ADD_PLAYER_PAKCET: {
        sc_add_player_packet* p = reinterpret_cast<sc_add_player_packet*>(packet);
        switch (p->slot) {
        case 0:
            MyLobbyReadyWidget->Number1OnOff->SetOpacity(1.0f);
            MyLobbyReadyWidget->Number1->SetText(FText::FromString(TEXT("Player 1")));
            MyLobbyReadyWidget->UpdateImageByClassType(EClassType::CT_Wind, MyLobbyReadyWidget->Number1Left);
            MyLobbyReadyWidget->UpdateImageByClassType(EClassType::CT_Fire, MyLobbyReadyWidget->Number1Right);
            break;

        case 1:
            MyLobbyReadyWidget->Number2OnOff->SetOpacity(1.0f);
            MyLobbyReadyWidget->Number2->SetText(FText::FromString(TEXT("Player 2")));
            MyLobbyReadyWidget->UpdateImageByClassType(EClassType::CT_Wind, MyLobbyReadyWidget->Number2Left);
            MyLobbyReadyWidget->UpdateImageByClassType(EClassType::CT_Fire, MyLobbyReadyWidget->Number2Right);
            break;

        case 2:
            MyLobbyReadyWidget->Number3OnOff->SetOpacity(1.0f);
            MyLobbyReadyWidget->Number3->SetText(FText::FromString(TEXT("Player 3")));
            MyLobbyReadyWidget->UpdateImageByClassType(EClassType::CT_Wind, MyLobbyReadyWidget->Number3Left);
            MyLobbyReadyWidget->UpdateImageByClassType(EClassType::CT_Fire, MyLobbyReadyWidget->Number3Right);
            break;

        case 3:
            MyLobbyReadyWidget->Number4OnOff->SetOpacity(1.0f);
            MyLobbyReadyWidget->Number4->SetText(FText::FromString(TEXT("Player 4")));
            MyLobbyReadyWidget->UpdateImageByClassType(EClassType::CT_Wind, MyLobbyReadyWidget->Number4Left);
            MyLobbyReadyWidget->UpdateImageByClassType(EClassType::CT_Fire, MyLobbyReadyWidget->Number4Right);
            break;
        }
        break;
    }

    case S2C_REMOVE_PLAYER_PAKCET: {
        sc_remove_player_packet* p = reinterpret_cast<sc_remove_player_packet*>(packet);
        FSlateBrush EmptyBrush;
        switch (p->slot) {
        case 0:
            MyLobbyReadyWidget->Number1OnOff->SetOpacity(0.25f);
            MyLobbyReadyWidget->Number1->SetText(FText::FromString(TEXT("Open")));
            MyLobbyReadyWidget->Number1Left->SetBrush(EmptyBrush);
            MyLobbyReadyWidget->Number1Right->SetBrush(EmptyBrush);
            break;

        case 1:
            MyLobbyReadyWidget->Number2OnOff->SetOpacity(0.25f);
            MyLobbyReadyWidget->Number2->SetText(FText::FromString(TEXT("Open")));
            MyLobbyReadyWidget->Number2Left->SetBrush(EmptyBrush);
            MyLobbyReadyWidget->Number2Right->SetBrush(EmptyBrush);
            break;

        case 2:
            MyLobbyReadyWidget->Number3OnOff->SetOpacity(0.25f);
            MyLobbyReadyWidget->Number3->SetText(FText::FromString(TEXT("Open")));
            MyLobbyReadyWidget->Number3Left->SetBrush(EmptyBrush);
            MyLobbyReadyWidget->Number3Right->SetBrush(EmptyBrush);
            break;

        case 3:
            MyLobbyReadyWidget->Number4OnOff->SetOpacity(0.25f);
            MyLobbyReadyWidget->Number4->SetText(FText::FromString(TEXT("Open")));
            MyLobbyReadyWidget->Number4Left->SetBrush(EmptyBrush);
            MyLobbyReadyWidget->Number4Right->SetBrush(EmptyBrush);
            break;
        }
        break;
    }

    case S2C_CHANGE_ELEMENT_PAKCET: {
        sc_change_element_packet* p = reinterpret_cast<sc_change_element_packet*>(packet);
        switch (p->slot) {
        case 0:
            if (p->is_left) {
                MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->element), MyLobbyReadyWidget->Number1Left);
                g_elements[0] = p->element;
            } else {
                MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->element), MyLobbyReadyWidget->Number1Right);
                g_elements[1] = p->element;
            }
            break;

        case 1:
            if (p->is_left) {
                MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->element), MyLobbyReadyWidget->Number2Left);
                g_elements[2] = p->element;
            } else {
                MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->element), MyLobbyReadyWidget->Number2Right);
                g_elements[3] = p->element;
            }
            break;

        case 2:
            if (p->is_left) {
                MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->element), MyLobbyReadyWidget->Number3Left);
                g_elements[4] = p->element;
            } else {
                MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->element), MyLobbyReadyWidget->Number3Right);
                g_elements[5] = p->element;
            }
            break;

        case 3:
            if (p->is_left) {
                MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->element), MyLobbyReadyWidget->Number4Left);
                g_elements[6] = p->element;
            } else {
                MyLobbyReadyWidget->UpdateImageByClassType(static_cast<EClassType>(p->element), MyLobbyReadyWidget->Number4Right);
                g_elements[7] = p->element;
            }
            break;
        }
        break;
    }

    case S2C_START_GAME_PAKCET: {
        {
            sc_start_game_packet* p = reinterpret_cast<sc_start_game_packet*>(packet);
            g_num_of_players = p->num_of_players;
        }

        APlayerController* PC = MyLobbyReadyWidget->GetWorld()->GetFirstPlayerController();

        if (PC)
        {
            // 마우스 커서 숨기기
            PC->bShowMouseCursor = false;

            // 입력 모드를 게임 플레이 모드로 전환 (게임 및 UI 입력 모두 허용)
            FInputModeGameOnly InputMode;
            PC->SetInputMode(InputMode);
        }

        // 월드 컨텍스트 가져오기
        UWorld* World = MyLobbyReadyWidget->GetWorld();

        if (World)
        {
            // 레벨 경로는 프로젝트 내 경로 (패키지 경로)
            FString LevelName = TEXT("DemoLevel1");

            UGameplayStatics::OpenLevel(World, FName(*LevelName));
        }

        //MyPlayerController->InitSocket();

        if (g_is_host) {
            cs_start_game_packet p;
            p.packet_size = sizeof(cs_start_game_packet);
            p.packet_type = C2S_START_GAME_PAKCET;
            g_do_send(&p);
        }
        break;
    }

    default:
        break;
    }
}

inline void g_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
    if ((0 != err) || (0 == num_bytes)) {
        // UE_LOG
        return;
    }

    // Process Packet
    char* p = g_recv_over.m_buffer;
    unsigned char packet_size = p[0];
    int remained = g_remained + num_bytes;

    while (packet_size <= remained) {
        g_process_packet(p);
        p += packet_size;
        remained -= packet_size;
        if (!remained) {
            break;
        }
        packet_size = p[0];
    }

    g_remained = remained;
    if (remained) {
        memcpy(g_recv_over.m_buffer, p, remained);
    }

    DWORD recv_bytes;
    DWORD recv_flag = 0;
    g_recv_over.m_wsabuf[0].buf = g_recv_over.m_buffer + g_remained;
    g_recv_over.m_wsabuf[0].len = sizeof(g_recv_over.m_buffer) - g_remained;
    WSARecv(g_l_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, g_recv_callback);
}

inline void g_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
    EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
    delete p;
}
