// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include <array>
#include <thread>
#include <chrono>
#include <atomic>
#include "SESSION.h"

constexpr short HOST_PORT = 3000;
constexpr char HOST_ADDRESS[] = "127.0.0.1";

//////////////////////////////////////////////////
// Server
SOCKET g_s_socket;
std::array<std::unique_ptr<SESSION>, MAX_CLIENTS> g_clients;
std::thread g_s_thread;
std::atomic<bool> g_is_host;
std::atomic<bool> g_running;

void server_thread();
void accept_thread();
void h_process_packet(char* p);
void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

//////////////////////////////////////////////////
// Client
EXP_OVER g_recv_over;
int g_remained;
std::array<APlayerCharacter*, MAX_CLIENTS> g_players;

void c_process_packet(char* p);
void CALLBACK c_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

//////////////////////////////////////////////////
// AMyPlayerController
void AMyPlayerController::BeginPlay() {
	Super::BeginPlay();
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
	g_running = true;

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
	else { UE_LOG(LogTemp, Warning, TEXT("Host Socket Created Succeed")); }

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
	g_running = false;

	if (g_is_host) {
		// Dummy Socket
		SOCKET d_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
		SOCKADDR_IN addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(HOST_PORT);
		inet_pton(AF_INET, HOST_ADDRESS, &addr.sin_addr);
		WSAConnect(d_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
		closesocket(d_socket);

		// Join Server Thread
		g_s_thread.join();

		// Delete SESSION
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_clients[client_id]) {
				g_clients[client_id] = nullptr;
			}
		}
	}

	// Delete Player
	for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
		if (g_players[client_id]) {
			g_players[client_id]->Destroy();
			g_players[client_id] = nullptr;
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

	while (g_running) {
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
		if (50 <= exec_ms) {
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
	while (true) {
		// WSAAccept
		auto c_socket = WSAAccept(g_s_socket, reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == c_socket) { UE_LOG(LogTemp, Warning, TEXT("WSAAccept Failed")); }
		else { UE_LOG(LogTemp, Warning, TEXT("WSAAccept Succeed")); }

		if (!g_running) {
			closesocket(c_socket);
			break;
		}

		// Create SESSION
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (!g_clients[client_id]) {
				g_clients[client_id] = std::make_unique<SESSION>(client_id, c_socket, h_recv_callback);
				UE_LOG(LogTemp, Warning, TEXT("[Host] Player %d Connected to Server"), g_clients[client_id]->m_id);

				// Send Player Info to Player
				hc_player_info_packet p;
				SESSION* c = g_clients[client_id].get();
				p.packet_size = sizeof(hc_player_info_packet);
				p.packet_type = H2C_PLAYER_INFO_PACKET;
				p.id = c->m_id;
				p.yaw = c->m_yaw;
				p.vx = c->m_velocity.X; p.vy = c->m_velocity.Y; p.vz = c->m_velocity.Z;
				p.hp = c->m_hp;
				p.animation_state = c->m_animation_state;
				p.current_element = c->m_current_element;
				g_clients[client_id]->do_send(&p);

				// Send Player Info to Others
				p.packet_type = H2C_PLAYER_ENTER_PACKET;
				for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
					if (client_id != other_id) {
						if (g_clients[other_id]) {
							g_clients[other_id]->do_send(&p);
						}
					}
				}

				// Send Other Infos to Player
				for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
					if (client_id != other_id) {
						if (g_clients[other_id]) {
							c = g_clients[other_id].get();
							p.id = c->m_id;
							p.yaw = c->m_yaw;
							p.vx = c->m_velocity.X; p.vy = c->m_velocity.Y; p.vz = c->m_velocity.Z;
							p.hp = c->m_hp;
							p.animation_state = c->m_animation_state;
							p.current_element = c->m_current_element;
							g_clients[client_id]->do_send(&p);
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
	UE_LOG(LogTemp, Warning, TEXT("[Host] Received Packet Type : %d"), packet_type);
	switch (packet_type) {
	case C2H_PLAYER_VECTOR_PACKET: {
		player_vector_packet* p = reinterpret_cast<player_vector_packet*>(packet);
		p->packet_type = H2C_PLAYER_VECTOR_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_clients[other_id]) {
					g_clients[other_id]->do_send(p);
					UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_STOPPED_PACKET: {
		player_stopped_packet* p = reinterpret_cast<player_stopped_packet*>(packet);
		p->packet_type = H2C_PLAYER_STOPPED_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_clients[other_id]) {
					g_clients[other_id]->do_send(p);
					UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Stopped Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_DIRECTION_PACKET: {
		player_direction_packet* p = reinterpret_cast<player_direction_packet*>(packet);
		p->packet_type = H2C_PLAYER_DIRECTION_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_clients[other_id]) {
					g_clients[other_id]->do_send(p);
					UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Direction Packet to Player %d"), p->id, other_id);
				}
			}
		}
	}
	}
}

void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	SESSION* o = reinterpret_cast<SESSION*>(p_over);
	if (!o) { return; }
	UE_LOG(LogTemp, Warning, TEXT("[Host] Received Packet from Player %d"), o->m_id);

	if ((0 != err) || (0 == num_bytes)) {
		UE_LOG(LogTemp, Warning, TEXT("[Host] Player %d DIsconnected"), o->m_id);

		hc_player_leave_packet p;
		p.packet_size = sizeof(hc_player_leave_packet);
		p.packet_type = H2C_PLAYER_LEAVE_PACKET;
		p.id = g_clients[o->m_id]->m_id;

		g_clients[p.id] = nullptr;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (g_clients[other_id]) {
				g_clients[other_id]->do_send(&p);
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


void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}

//////////////////////////////////////////////////
// Client CALLBACK
void c_process_packet(char* packet) {
	char packet_type = packet[1];
	UE_LOG(LogTemp, Warning, TEXT("[Client] Received Packet Type : %d"), packet_type);
	switch (packet_type) {
	case H2C_PLAYER_INFO_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);

		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;

		APlayerCharacter* MyPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
		if (MyPlayer) {
			MyPlayer->set_id(p->id);
			g_players[p->id] = MyPlayer;

			UE_LOG(LogTemp, Warning, TEXT("[Client] Registered Local Player (ID: %d) in g_players"), p->id);
		}
		break;
	}

	case H2C_PLAYER_ENTER_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);
		UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d has Entered the Game"), p->id);

		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;

		FVector SpawnLocation(40'000, -40'000, 1'500);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APlayerCharacter* NewPlayer = World->SpawnActor<APlayerCharacter>(APlayerCharacter::StaticClass(), SpawnLocation, SpawnRotation, Params);

		if (NewPlayer) {
			NewPlayer->AutoPossessPlayer = EAutoReceiveInput::Disabled;
			NewPlayer->DisableInput(nullptr);
			NewPlayer->set_id(p->id);

			g_players[p->id] = NewPlayer;

			UE_LOG(LogTemp, Warning, TEXT("[Client] Spawned Player %d and Stored in g_players"), p->id);
		}
		break;
	}

	case H2C_PLAYER_LEAVE_PACKET: {
		hc_player_leave_packet* p = reinterpret_cast<hc_player_leave_packet*>(packet);
		UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d has Leaved the Game"), p->id);

		g_players[p->id]->Destroy();
		g_players[p->id] = nullptr;
		break;
	}

	case H2C_PLAYER_VECTOR_PACKET: {
		player_vector_packet* p = reinterpret_cast<player_vector_packet*>(packet);
		FVector Position = FVector(p->x, p->y, p->z);
		g_players[p->id]->SetActorLocation(Position, false);
		g_players[p->id]->set_velocity(p->vx, p->vy, p->vz);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d Moved, vx : %.2f, vy : %.2f, vz : %.2f"), p->id, p->vx, p->vy, p->vz);
		break;
	}

	case H2C_PLAYER_STOPPED_PACKET: {
		player_stopped_packet* p = reinterpret_cast<player_stopped_packet*>(packet);
		g_players[p->id]->set_velocity(0.0f, 0.0f, 0.0f);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d Stopped"), p->id);
		break;
	}

	case H2C_PLAYER_DIRECTION_PACKET: {
		player_direction_packet* p = reinterpret_cast<player_direction_packet*>(packet);
		g_players[p->id]->rotate(p->yaw);
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
	UE_LOG(LogTemp, Warning, TEXT("[Client] Received Packet from Host"));
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
	WSARecv(g_h_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, c_recv_callback);
}

void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}
