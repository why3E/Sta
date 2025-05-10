// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "MySkillBase.h"
#include "MyWindCutter.h"
#include "MyWindSkill.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Blueprint/UserWidget.h"

#include "SESSION.h"

#include <array>
#include <atomic>
#include <chrono>
#include <thread>

constexpr short HOST_PORT = 3000;
constexpr char HOST_ADDRESS[] = "127.0.0.1";

//////////////////////////////////////////////////
// Server
SOCKET g_s_socket;
std::thread g_s_thread;
std::atomic<bool> g_s_running;
std::atomic<unsigned short> g_s_skill_cnt;

std::unordered_map<unsigned short, ACharacter*> g_s_monsters;
std::array<std::unique_ptr<SESSION>, MAX_CLIENTS> g_s_clients;

void server_thread();
void accept_thread();
void h_process_packet(char* p);
extern void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
extern void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

//////////////////////////////////////////////////
// Client
EXP_OVER g_recv_over;
int g_remained;
std::array<APlayerCharacter*, MAX_CLIENTS> g_c_players;

void c_process_packet(char* p);
void CALLBACK c_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

//////////////////////////////////////////////////
// AMyPlayerController
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // 위젯 클래스 로드
    UClass* HUDWidgetClass = LoadClass<UUserWidget>(
        nullptr,
        TEXT("/Game/HUD/WBP_HUD.WBP_HUD_C")
    );

    if (HUDWidgetClass)
    {
        // 위젯 생성 및 뷰포트에 추가
        UUserWidget* HUD = CreateWidget<UUserWidget>(this, HUDWidgetClass);
        if (HUD)
        {
            HUD->AddToViewport();
            UE_LOG(LogTemp, Warning, TEXT("HUD Widget successfully added to viewport."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create HUD Widget."));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load HUD Widget class."));
    }

    InitSocket();
}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupSocket();
	Super::EndPlay(EndPlayReason);
}

//////////////////////////////////////////////////
// Socket
void AMyPlayerController::InitSocket()
{
	g_is_host = true;
	g_skills.clear();
	g_collisions.clear();
	g_monsters.clear();

	g_s_running = true;
	g_s_skill_cnt = 0;

	// [Client] : WSAStartup
	WSADATA WSAData;
	auto ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret) { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Succeed")); }

	g_h_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == g_h_socket) { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Succeed")); }

	// [Host] : WSASocket
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == g_s_socket) { UE_LOG(LogTemp, Warning, TEXT("Host Socket Create Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Host Socket Create Succeed")); }

	u_long noblock = 1;
	ioctlsocket(g_s_socket, FIONBIO, &noblock);

	// Bind & Listen
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(g_s_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == ret) { 
		UE_LOG(LogTemp, Warning, TEXT("Bind Failed : %d"), WSAGetLastError()); 
		g_is_host = false;
		closesocket(g_s_socket);
	}
	else { 
		UE_LOG(LogTemp, Warning, TEXT("Bind Succeed")); 
		g_is_host = true;
	}

	if (g_is_host) {
		g_s_thread = std::thread(server_thread);
	}

	// Connect to host
	inet_pton(AF_INET, HOST_ADDRESS, &addr.sin_addr);

	ret = WSAConnect(g_h_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == ret) { UE_LOG(LogTemp, Warning, TEXT("WSAConnect Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("WSAConnect Succeed")); }

	DWORD recv_bytes;
	DWORD recv_flag = 0;
	ret = WSARecv(g_h_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, c_recv_callback);
}

void AMyPlayerController::CleanupSocket()
{
	g_s_running = false;

	if (g_is_host) {
		// Join Server Thread
		g_s_thread.join();

		g_s_monsters.clear();

		// Delete SESSION
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id] = nullptr;
			}
		}
	}

	// Delete Player
	for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
		if (g_c_players[client_id]) {
			g_c_players[client_id]->Destroy();
			g_c_players[client_id] = nullptr;
		}
	}

	closesocket(g_h_socket);
	WSACleanup();
}

//////////////////////////////////////////////////
// Server Thread
void server_thread() {
	// Create Accept Thread
	std::thread a_thread(accept_thread);

	auto last_update_t = std::chrono::system_clock::now();
	auto last_packet_t = std::chrono::system_clock::now();

	while (g_s_running) {
		// Game Logic
		auto curr_t = std::chrono::system_clock::now();
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_update_t).count();

		if (16 <= exec_ms) {
			/*
				Game Logic
			*/

			last_update_t = std::chrono::system_clock::now();
		}

		// WSASend
		exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_packet_t).count();
		if (16 <= exec_ms) {
			/*
				WSASend
			*/

			last_packet_t = std::chrono::system_clock::now();
		}
	}

	a_thread.join();
}

//////////////////////////////////////////////////
// Accept Thread
void accept_thread() {
	auto ret = listen(g_s_socket, SOMAXCONN);
	if (SOCKET_ERROR == ret) { UE_LOG(LogTemp, Warning, TEXT("Listen Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Listen Succeed")); }

	SOCKADDR_IN addr;
	INT addr_size = sizeof(SOCKADDR_IN);

	// Accept Loop
	while (g_s_running) {
		// WSAAccept
		auto c_socket = WSAAccept(g_s_socket, reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == c_socket) { 
			int err = WSAGetLastError();
			if (WSAEWOULDBLOCK == err) {
				SleepEx(1, TRUE); 
				continue;
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("WSAAccept Failed : %d"), err);
				break;
			}
		}
		else { UE_LOG(LogTemp, Warning, TEXT("WSAAccept Succeed")); }

		// Create SESSION
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (!g_s_clients[client_id]) {
				g_s_clients[client_id] = std::make_unique<SESSION>(client_id, c_socket, h_recv_callback, h_send_callback);
				UE_LOG(LogTemp, Warning, TEXT("[Host] Player %d Connected to Server"), g_s_clients[client_id]->m_id);

				// Send Player Info to Player
				hc_player_info_packet p;
				SESSION* c = g_s_clients[client_id].get();
				p.packet_size = sizeof(hc_player_info_packet);
				p.packet_type = H2C_PLAYER_INFO_PACKET;
				p.id = c->m_id;
				p.yaw = c->m_yaw;
				p.vx = c->m_velocity.X; p.vy = c->m_velocity.Y; p.vz = c->m_velocity.Z;
				p.hp = c->m_hp;
				p.current_element = c->m_current_element;
				g_s_clients[client_id]->do_send(&p);

				// Send Player Info to Others
				p.packet_type = H2C_PLAYER_ENTER_PACKET;
				for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
					if (client_id != other_id) {
						if (g_s_clients[other_id]) {
							g_s_clients[other_id]->do_send(&p);
						}
					}
				}

				// Send Other Infos to Player
				for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
					if (client_id != other_id) {
						if (g_s_clients[other_id]) {
							c = g_s_clients[other_id].get();
							p.id = c->m_id;
							p.yaw = c->m_yaw;
							p.vx = c->m_velocity.X; p.vy = c->m_velocity.Y; p.vz = c->m_velocity.Z;
							p.hp = c->m_hp;
							p.current_element = c->m_current_element;
							g_s_clients[client_id]->do_send(&p);
						}
					}
				}
				break;
			}
		}
	}

	closesocket(g_s_socket);
}





//////////////////////////////////////////////////
// Host CALLBACK
void h_process_packet(char* packet) {
	char packet_type = packet[1];
	//UE_LOG(LogTemp, Warning, TEXT("[Host] Received Packet Type : %d"), packet_type);

	switch (packet_type) {
	case C2H_PLAYER_VECTOR_PACKET: {
		player_vector_packet* p = reinterpret_cast<player_vector_packet*>(packet);
		p->packet_type = H2C_PLAYER_VECTOR_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Vector Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_STOP_PACKET: {
		player_stop_packet* p = reinterpret_cast<player_stop_packet*>(packet);
		p->packet_type = H2C_PLAYER_STOP_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Stop Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_ROTATE_PACKET: {
		player_rotate_packet* p = reinterpret_cast<player_rotate_packet*>(packet);
		p->packet_type = H2C_PLAYER_ROTATE_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Rotation Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_JUMP_PACKET: {
		player_jump_packet* p = reinterpret_cast<player_jump_packet*>(packet);
		p->packet_type = H2C_PLAYER_JUMP_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Jump Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_SKILL_VECTOR_PACKET: {
		ch_player_skill_vector_packet* ch_p = reinterpret_cast<ch_player_skill_vector_packet*>(packet);
		hc_player_skill_vector_packet hc_p;
		hc_p.packet_size = sizeof(hc_player_skill_vector_packet);
		hc_p.packet_type = H2C_PLAYER_SKILL_VECTOR_PACKET;
		hc_p.player_id = ch_p->player_id;
		hc_p.skill_id = g_s_skill_cnt++;
		hc_p.skill_type = ch_p->skill_type;
		hc_p.x = ch_p->x; hc_p.y = ch_p->y; hc_p.z = ch_p->z;
		hc_p.is_left = ch_p->is_left;
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(&hc_p);
				//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Skill Packet to Player %d"), ch_p->player_id, client_id);
			}
		}
		break;
	}

	case C2H_PLAYER_SKILL_ROTATOR_PACKET: {
		ch_player_skill_rotator_packet* ch_p = reinterpret_cast<ch_player_skill_rotator_packet*>(packet);
		hc_player_skill_rotator_packet hc_p;
		hc_p.packet_size = sizeof(hc_player_skill_rotator_packet);
		hc_p.packet_type = H2C_PLAYER_SKILL_ROTATOR_PACKET;
		hc_p.player_id = ch_p->player_id;

		switch (ch_p->skill_type) {
		case SKILL_FIRE_WALL:
			hc_p.skill_id = g_s_skill_cnt.fetch_add(5);
			break;

		default:
			hc_p.skill_id = g_s_skill_cnt++;
			break;
		}

		hc_p.skill_type = ch_p->skill_type;
		hc_p.x = ch_p->x; hc_p.y = ch_p->y; hc_p.z = ch_p->z;
		hc_p.pitch = ch_p->pitch; hc_p.yaw = ch_p->yaw; hc_p.roll = ch_p->roll;
		hc_p.is_left = ch_p->is_left;
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(&hc_p);
				//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Skill Packet to Player %d"), p->player_id, client_id);
			}
		}
		break;
	}

	case C2H_PLAYER_CHANGE_ELEMENT_PACKET: {
		player_change_element_packet* p = reinterpret_cast<player_change_element_packet*>(packet);
		p->packet_type = H2C_PLAYER_CHANGE_ELEMENT_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Skill Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_COLLISION_PACKET: {
		collision_packet* p = reinterpret_cast<collision_packet*>(packet);
		p->packet_type = H2C_COLLISION_PACKET;
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(p);
			}
		}
		//UE_LOG(LogTemp, Warning, TEXT("[Host] Skill %d and %d Collision"), p->attacker_id, p->victim_id);
		break;
	}

	case C2H_SKILL_CREATE_PACKET: {
		ch_skill_create_packet* ch_p = reinterpret_cast<ch_skill_create_packet*>(packet);
		hc_skill_create_packet hc_p;
		hc_p.packet_size = sizeof(hc_skill_create_packet);
		hc_p.packet_type = H2C_SKILL_CREATE_PACKET;
		hc_p.skill_type = ch_p->skill_type;
		hc_p.old_skill_id = ch_p->old_skill_id;
		hc_p.new_skill_id = g_s_skill_cnt++;
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(&hc_p);
			}
		}
		//UE_LOG(LogTemp, Warning, TEXT("[Host] Skill %d Create"), p->new_skill_id);
		break;
	}
	}
}

extern void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	//UE_LOG(LogTemp, Warning, TEXT("[Host] h_recv_callback"));

	SESSION* o = reinterpret_cast<SESSION*>(p_over);
	if (!o) { return; }
	//UE_LOG(LogTemp, Warning, TEXT("[Host] Received Packet from Player %d"), o->m_id);

	if ((0 != err) || (0 == num_bytes)) {
		UE_LOG(LogTemp, Warning, TEXT("[Host] Player %d Disconnected"), o->m_id);

		hc_player_leave_packet p;
		p.packet_size = sizeof(hc_player_leave_packet);
		p.packet_type = H2C_PLAYER_LEAVE_PACKET;
		p.id = g_s_clients[o->m_id]->m_id;

		g_s_clients[p.id] = nullptr;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (g_s_clients[other_id]) {
				g_s_clients[other_id]->do_send(&p);
			}
		}
		return;
	}

	// Process Packet
	char* p = o->m_recv_over.m_buffer;
	unsigned char packet_size = p[0];
	int remained = o->m_remained + num_bytes;

	while (packet_size <= remained) {
		h_process_packet(p);
		p += packet_size;
		remained -= packet_size;
		if (!remained) {
			break;
		}
		packet_size = p[0];
	}

	o->m_remained = remained;
	if (remained) {
		memcpy(o->m_recv_over.m_buffer, p, remained);
	}

	o->do_recv();
}

extern void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	//UE_LOG(LogTemp, Warning, TEXT("[Host] h_send_callback"));
	
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}





//////////////////////////////////////////////////
// Client CALLBACK
void c_process_packet(char* packet) {
	char packet_type = packet[1];
	//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Packet Type : %d"), packet_type);

	switch (packet_type) {
	case H2C_PLAYER_INFO_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);

		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;

		APlayerCharacter* MyPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
		if (MyPlayer) {
			MyPlayer->set_id(p->id);
			MyPlayer->set_is_player(true);

			g_c_players[p->id] = MyPlayer;

			//UE_LOG(LogTemp, Warning, TEXT("[Client] Registered Local Player (ID: %d) in g_c_players"), p->id);
		}
		break;
	}

	case H2C_PLAYER_ENTER_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);
		UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d has Entered the Game"), p->id);
	
		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;
	
		FVector SpawnLocation(37'975.0f, -40'000.0f, 950.0f); // 초기 위치 설정
		FRotator SpawnRotation = FRotator::ZeroRotator;
	
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
		//APlayerCharacter* NewPlayer = World->SpawnActor<APlayerCharacter>(APlayerCharacter::StaticClass(), SpawnLocation, SpawnRotation, Params);
	
		UClass* PlayerBPClass = LoadClass<APlayerCharacter>(
			nullptr,
			TEXT("/Game/player_anim/MyPlayerCharacter.MyPlayerCharacter_C")
		);

		if (!PlayerBPClass) {
			UE_LOG(LogTemp, Error, TEXT("Failed to load BP_PlayerCharacter!"));
			return;
		}

		APlayerCharacter* NewPlayer = World->SpawnActor<APlayerCharacter>(
			PlayerBPClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

		NewPlayer->set_is_player(false);
		NewPlayer->set_id(p->id);

		AAIController* NewAI = World->SpawnActor<AAIController>(
			AAIController::StaticClass(),
			SpawnLocation,
			SpawnRotation,
			Params
		);

		if (NewAI) {
			NewAI->Possess(NewPlayer);
			UE_LOG(LogTemp, Warning, TEXT("AIController Possessed Avatar"));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn AIController"));
		}

		UClass* AnimClass = LoadClass<UAnimInstance>(
			nullptr,
			TEXT("/Game/player_anim/MyPlayerAnim.MyPlayerAnim_C")
		);
	
		if (AnimClass)
		{
			NewPlayer->GetMesh()->SetAnimInstanceClass(AnimClass);
			NewPlayer->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			NewPlayer->GetMesh()->SetComponentTickEnabled(true);
			NewPlayer->GetMesh()->bPauseAnims = false;
			NewPlayer->GetMesh()->bNoSkeletonUpdate = false;
	
			UE_LOG(LogTemp, Warning, TEXT("AnimInstance Set: %s"), *AnimClass->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to Load AnimBP"));
		}


		g_c_players[p->id] = NewPlayer;
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Spawned Player %d and Stored in g_c_players"), p->id);
		break;
	}

	case H2C_PLAYER_LEAVE_PACKET: {
		hc_player_leave_packet* p = reinterpret_cast<hc_player_leave_packet*>(packet);
		UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d has Leaved the Game"), p->id);

		g_c_players[p->id]->Destroy();
		g_c_players[p->id] = nullptr;
		break;
	}

	case H2C_PLAYER_VECTOR_PACKET: {
		player_vector_packet* p = reinterpret_cast<player_vector_packet*>(packet);
		FVector Position(p->x, p->y, p->z);
		FVector Velocity(p->vx, p->vy, p->vz);
		g_c_players[p->id]->SetActorLocation(Position, false);
		g_c_players[p->id]->set_velocity(Velocity.X, Velocity.Y, Velocity.Z);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d Moved, vx : %.2f, vy : %.2f"), p->id, p->vx, p->vy);
		break;
	}

	case H2C_PLAYER_STOP_PACKET: {
		player_stop_packet* p = reinterpret_cast<player_stop_packet*>(packet);
		FVector Position(p->x, p->y, p->z);
		g_c_players[p->id]->SetActorLocation(Position, false);
		g_c_players[p->id]->set_velocity(0.0f, 0.0f, 0.0f);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Stop Packet"), p->id);
		break;
	}

	case H2C_PLAYER_ROTATE_PACKET: {
		player_rotate_packet* p = reinterpret_cast<player_rotate_packet*>(packet);
		g_c_players[p->id]->rotate(p->yaw);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Rotation Packet"), p->id);
		break;
	}

	case H2C_PLAYER_JUMP_PACKET: {
		player_jump_packet* p = reinterpret_cast<player_jump_packet*>(packet);
		g_c_players[p->id]->LaunchCharacter(FVector(0, 0, 800), false, true);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Jump Packet"), p->id);
		break;
	}

	case H2C_PLAYER_SKILL_VECTOR_PACKET: {
		hc_player_skill_vector_packet* p = reinterpret_cast<hc_player_skill_vector_packet*>(packet);
		g_c_players[p->player_id]->use_skill(p->skill_id, p->skill_type, FVector(p->x, p->y, p->z), p->is_left);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Skill %d Packet"), p->player_id, p->skill_id);
		break;
	}

	case H2C_PLAYER_SKILL_ROTATOR_PACKET: {
		hc_player_skill_rotator_packet* p = reinterpret_cast<hc_player_skill_rotator_packet*>(packet);
		g_c_players[p->player_id]->use_skill(p->skill_id, p->skill_type, FVector(p->x, p->y, p->z), FRotator(p->pitch, p->yaw, p->roll), p->is_left);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Skill %d Packet"), p->player_id, p->skill_id);
		break;
	}

	case H2C_PLAYER_CHANGE_ELEMENT_PACKET: {
		player_change_element_packet* p = reinterpret_cast<player_change_element_packet*>(packet);
		g_c_players[p->id]->change_element(p->element_type, p->is_left);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Skill Packet"), p->player_id);
		break;
	}

	case H2C_COLLISION_PACKET: {
		collision_packet* p = reinterpret_cast<collision_packet*>(packet);
		switch (p->collision_type) {
		case SKILL_SKILL_COLLISION:
			if (g_skills.count(p->attacker_id) && g_skills.count(p->victim_id)) {
				g_skills[p->attacker_id]->Overlap(g_skills[p->victim_id]);
				g_skills[p->victim_id]->Overlap(g_skills[p->attacker_id]);
				//UE_LOG(LogTemp, Error, TEXT("[Client] Skill %d and %d Collision"), p->attacker_id, p->victim_id);
			} else {
				g_collisions[p->attacker_id].push(p->victim_id);
				g_collisions[p->victim_id].push(p->attacker_id);
				//UE_LOG(LogTemp, Error, TEXT("[Client] Skill %d and %d Delayed"), p->attacker_id, p->victim_id);
			}
			break;
		}
		break;
	}

	case H2C_SKILL_CREATE_PACKET: {
		hc_skill_create_packet* p = reinterpret_cast<hc_skill_create_packet*>(packet);
		switch (p->skill_type) {
		case SKILL_WIND_FIRE_BOMB:
			if (g_skills.count(p->old_skill_id)) {
				if (g_skills[p->old_skill_id]->IsA(AMyWindCutter::StaticClass())) {
					Cast<AMyWindCutter>(g_skills[p->old_skill_id])->MixBombAttack(EClassType::CT_Fire, p->new_skill_id);
				}
			}
			break;

		case SKILL_WIND_WIND_TORNADO:
			if (g_skills.count(p->old_skill_id)) {
				if (g_skills[p->old_skill_id]->IsA(AMyWindSkill::StaticClass())) {
					Cast<AMyWindSkill>(g_skills[p->old_skill_id])->SpawnMixTonado(p->new_skill_id);
				}
			}
			break;
		}
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Collision Packet"));
		break;
	}
	}
}

void CALLBACK c_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	if ((0 != err) || (0 == num_bytes)) {
		UE_LOG(LogTemp, Warning, TEXT("[Client] Server Disconnected"));
		return;
	}

	// Process Packet
	//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Packet from Host"));
	char* p = g_recv_over.m_buffer;
	unsigned char packet_size = p[0];
	int remained = g_remained + num_bytes;

	while (packet_size <= remained) {
		c_process_packet(p);
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
	WSARecv(g_h_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, c_recv_callback);
}

void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}
