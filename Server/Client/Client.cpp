#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <queue>
#include "..\Common.h"
#include "SESSION.h"

#pragma comment (lib, "WS2_32.LIB")

constexpr short HOST_PORT = 3000;
constexpr char HOST_ADDRESS[] = "127.0.0.1";

volatile bool g_is_game_running = false;
std::mutex g_q_lock;
std::unordered_map<int, SESSION> g_clients;
std::queue<ch_key_packet> g_input_queue;

void server_thread();
void accept_thread();
void play_game();

// Use this code until Lobby code is implemented
int main() {
	// WSAStartup
	WSADATA WSAData;
	auto ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret) err_display("WSAStartup");

	// Create Socket
	SOCKET h_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == h_socket) err_display("WSASocket");

	// If host, create Server Thread
	bool is_host;
	std::cout << "Host : 1, Client : 0" << std::endl;
	std::cout << "Enter : ";
	std::cin >> is_host;

	std::thread s_thread;
	if (is_host) {
		s_thread = std::thread(server_thread);
	}

	// Connect to host
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT);
	inet_pton(AF_INET, HOST_ADDRESS, &addr.sin_addr);
	ret = WSAConnect(h_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == ret) err_display("WSAConnect");
	
	// Game Loop
	play_game();
	
	// If host, wait until Server Thread finishes
	if (is_host) {
		s_thread.join();
	}

	// WSACleanup
	closesocket(h_socket);
	WSACleanup();
}

void server_thread() {
	// Create Accept Thread
	std::thread a_thread(accept_thread);

	auto last_update_t = std::chrono::system_clock::now();
	auto last_packet_t = std::chrono::system_clock::now();

	while (g_is_game_running) {
		// Apply Input
		{
			std::lock_guard<std::mutex> lock(g_q_lock);
			while (!g_input_queue.empty()) {
				/* Apply input */
			}
		}

		// Game Logic
		auto curr_t = std::chrono::system_clock::now();
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_update_t).count();

		if (16 <= exec_ms) {
			/* Game Logic */

			last_update_t = std::chrono::system_clock::now();
		}

		// WSASend
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_packet_t).count();
		if (50 <= exec_ms) {
			for (auto& client : g_clients) {
				/* WSASend */

				last_packet_t = std::chrono::system_clock::now();
			}
		}
	}
}

void accept_thread() {

}

void play_game() {

}