// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "PlayerCharacter.h"
#include <array>
#include <thread>
#include <chrono>
#include <atomic>
#include "SESSION.h"

constexpr short HOST_PORT = 3000;
constexpr char HOST_ADDRESS[] = "127.0.0.1";

//////////////////////////////////////////////////
// Server
std::array<std::unique_ptr<SESSION>, MAX_CLIENTS> g_clients;
std::thread g_s_thread;
std::atomic<bool> g_running;

void server_thread();
void accept_thread();
void h_process_packet(char* p);
void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

//////////////////////////////////////////////////
// Client
EXP_OVER g_recv_over;
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

	// WSAStartup
	WSADATA WSAData;
	auto ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret) { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Succeed")); }

	g_h_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == g_h_socket) { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Succeed")); }

	//g_s_thread = std::thread(server_thread);

	// Connect to host
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT);
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

	//// Dummy Socket
	//SOCKET d_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	//SOCKADDR_IN addr;
	//addr.sin_family = AF_INET;
	//addr.sin_port = htons(HOST_PORT);
	//inet_pton(AF_INET, HOST_ADDRESS, &addr.sin_addr);
	//WSAConnect(d_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
	//closesocket(d_socket);

	//// Join Server Thread
	//if (g_s_thread.joinable())
	//	g_s_thread.join();

	// Shutdown Socket
	shutdown(g_h_socket, SD_BOTH);
	for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
		if (g_clients[client_id]) {
			// Graceful shutdown to cancel pending I/O
			shutdown(g_clients[client_id]->m_c_socket, SD_BOTH);
		}
	}

	// Sleep to Give Time for I/O Callbacks to Complete
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	// Delete SESSION
	for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
		if (g_clients[client_id]) {
			closesocket(g_clients[client_id]->m_c_socket);
			g_clients[client_id] = nullptr;
		}

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
	// WSASocket
	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == s_socket) { UE_LOG(LogTemp, Warning, TEXT("Host Socket Create Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Host Socket Created Succeed")); }

	// Bind & Listen
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	INT addr_size = sizeof(SOCKADDR_IN);

	auto ret = bind(s_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == ret) { UE_LOG(LogTemp, Warning, TEXT("Bind Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Bind Succeed")); }

	ret = listen(s_socket, SOMAXCONN);
	if (SOCKET_ERROR == ret) { UE_LOG(LogTemp, Warning, TEXT("Listen Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Listen Succeed")); }

	// Accept Loop
	while (true) {
		// WSAAccept
		auto c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);
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

				// Send Player Info to Player
				hc_player_info_packet p;
				SESSION* c = g_clients[client_id].get();
				p.packet_size = sizeof(hc_player_info_packet);
				p.packet_type = H2C_PLAYER_INFO_PACKET;
				p.id = c->m_id;
				p.x = c->m_x; p.y = c->m_y; p.z = c->m_z;
				p.dx = c->m_dx; p.dy = c->m_dy; p.dz = c->m_dz;
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
							p.x = c->m_x; p.y = c->m_y; p.z = c->m_z;
							p.dx = c->m_dx; p.dy = c->m_dy; p.dz = c->m_dz;
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

	closesocket(s_socket);
}

//////////////////////////////////////////////////
// Host CALLBACK
void h_process_packet(char* packet) {
	char packet_type = packet[1];
	UE_LOG(LogTemp, Warning, TEXT("[Server] Packet Type : %d"), packet_type);
	switch (packet_type) {
	case C2H_PLAYER_VECTOR_PACKET:
		player_vector_packet* p = reinterpret_cast<player_vector_packet*>(packet);
		p->packet_type = H2C_PLAYER_VECTOR_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			//if (p->id != other_id) {
				if (g_clients[other_id]) {
					g_clients[other_id]->do_send(p);
					UE_LOG(LogTemp, Warning, TEXT("[Server] Send Packet to Player %d"), other_id);
				}
			//}
		}
		break;
	}
}

void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	SESSION* o = reinterpret_cast<SESSION*>(p_over);
	UE_LOG(LogTemp, Warning, TEXT("[Server] Received Packet from Player %d"), o->m_id);

	if ((0 != err) || (0 == num_bytes)) {
		UE_LOG(LogTemp, Warning, TEXT("[Server] Player %d DIsconnected"), o->m_id);

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
	}

	char* p = o->m_recv_over.m_buffer;
	h_process_packet(p);

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
	UE_LOG(LogTemp, Warning, TEXT("[Client] Packet Type : %d"), packet_type);
	switch (packet_type) {
	case H2C_PLAYER_INFO_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);
		g_id = p->id;
		g_x = p->x; g_y = p->y; g_z = p->z;
		g_dx = p->dx; g_dy = p->dy; g_dz = p->dz;
		g_hp = p->hp;
		g_animation_state = p->animation_state;
		g_current_element = p->current_element;
		break;
	}

	case H2C_PLAYER_ENTER_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);
		UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d has Entered the Game"), p->id);

		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;

		FVector SpawnLocation(0, 0, 100.0f);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		APlayerCharacter* NewPlayer = World->SpawnActor<APlayerCharacter>(APlayerCharacter::StaticClass(), SpawnLocation, SpawnRotation, Params);

		if (NewPlayer) {
			NewPlayer->AutoPossessPlayer = EAutoReceiveInput::Disabled;
			NewPlayer->DisableInput(nullptr);

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
		UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d, dx : %.2f, dy : %.2f"), p->id, p->dx, p->dy);
		break;
	}
	}
}

void CALLBACK c_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	if ((0 != err) || (0 == num_bytes)) UE_LOG(LogTemp, Warning, TEXT("[Client] Server Disconnected"));

	char* p = g_recv_over.m_buffer;
	c_process_packet(p);

	DWORD recv_bytes;
	DWORD recv_flag = 0;
	WSARecv(g_h_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, c_recv_callback);
}

void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}
