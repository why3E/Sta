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

std::mutex g_q_lock;
std::queue<ch_key_packet> g_input_queue;
std::array<std::unique_ptr<SESSION>, MAX_CLIENTS> g_clients;

std::thread g_s_thread;
std::atomic<bool> g_running;

EXP_OVER g_recv_over;
std::array<std::unique_ptr<APlayerCharacter>, MAX_CLIENTS> g_players;

void server_thread();
void accept_thread();
void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

void process_packet(char* p);
void CALLBACK c_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

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
	for (auto& p : g_clients)
		p = nullptr;

	// WSAStartup
	WSADATA WSAData;
	auto ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret) { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Succeed")); }

	g_h_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == g_h_socket) { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Succeed")); }

	g_s_thread = std::thread(server_thread);

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

	// Dummy Socket
	SOCKET d_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT);
	inet_pton(AF_INET, HOST_ADDRESS, &addr.sin_addr);
	WSAConnect(d_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
	closesocket(d_socket);

	g_s_thread.join();

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
		// Apply Input
		{
			std::lock_guard<std::mutex> lock(g_q_lock);
			while (!g_input_queue.empty()) {
				/*
					Apply input
				*/
			}
		}

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
	while (g_running) {
		// WSAAccept
		auto c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == c_socket) { UE_LOG(LogTemp, Warning, TEXT("WSAAccept Failed : %d"), WSAGetLastError()); }
		else { UE_LOG(LogTemp, Warning, TEXT("WSAAccept Succeed")); }

		// Create SESSION
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (!g_clients[client_id]) {
				g_clients[client_id] = std::make_unique<SESSION>(client_id, c_socket, h_recv_callback);
				
				// Send Player Info to Player
				hc_player_packet p;
				SESSION* c = g_clients[client_id].get();
				p.packet_size = sizeof(hc_player_packet);
				p.packet_type = H2C_PLAYER_INFO_PACKET;
				p.id = c->m_id;
				p.x = c->m_x;
				p.y = c->m_y;
				p.z = c->m_z;
				p.dx = c->m_dx;
				p.dy = c->m_dy;
				p.dz = c->m_dz;
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
							p.x = c->m_x;
							p.y = c->m_y;
							p.z = c->m_z;
							p.dx = c->m_dx;
							p.dy = c->m_dy;
							p.dz = c->m_dz;
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
void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	if ((0 != err) || (0 == num_bytes)) UE_LOG(LogTemp, Warning, TEXT("h_recv_callback Failed : %d"), WSAGetLastError());

	SESSION* o = reinterpret_cast<SESSION*>(p_over);
	o->recv_callback(num_bytes, p_over, g_q_lock, g_input_queue);
}

void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}

//////////////////////////////////////////////////
// Client CALLBACK
void process_packet(char* p) {
	char packet_type = p[1];
	switch (packet_type) {
	case H2C_PLAYER_INFO_PACKET: {
		hc_player_packet* packet = reinterpret_cast<hc_player_packet*>(p);
		break;
	}

	case H2C_PLAYER_ENTER_PACKET: {
		hc_player_packet* packet = reinterpret_cast<hc_player_packet*>(p);
		UE_LOG(LogTemp, Warning, TEXT("Player %d has Entered the Game"), packet->id);
		break;
	}
	}
}

void CALLBACK c_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	if ((0 != err) || (0 == num_bytes)) UE_LOG(LogTemp, Warning, TEXT("c_recv_callback Failed : %d"), WSAGetLastError());

	char* p = g_recv_over.m_buffer;
	process_packet(p);

	DWORD recv_bytes;
	DWORD recv_flag = 0;
	WSARecv(g_h_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, c_recv_callback);
}

void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}
