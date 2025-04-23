#include <thread>
#include <chrono>
#include <array>
#include "SESSION.h"

#pragma comment (lib, "WS2_32.LIB")

constexpr short HOST_PORT = 3000;
constexpr char HOST_ADDRESS[] = "127.0.0.1";

volatile bool g_is_game_running = true;
std::mutex g_q_lock;
std::array<std::unique_ptr<SESSION>, MAX_CLIENT> g_clients;
std::queue<ch_key_packet> g_input_queue;

void server_thread();
void accept_thread();
void play_game();

void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

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
	std::cout << std::endl;

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

//////////////////////////////////////////////////
// Server Thread
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
		exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_packet_t).count();
		if (50 <= exec_ms) {
			for (auto& client : g_clients) {
				/* WSASend */

				last_packet_t = std::chrono::system_clock::now();
			}
		}
	}

	a_thread.join();
}

//////////////////////////////////////////////////
// Accept Thread
void accept_thread() {
	// WSASocket
	SOCKET s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == s_socket) err_display("WSASocket");

	// Bind & Listen
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	INT addr_size = sizeof(SOCKADDR_IN);

	auto ret = bind(s_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == ret) err_display("bind");

	ret = listen(s_socket, SOMAXCONN);
	if (SOCKET_ERROR == ret) err_display("listen");

	// Accept Loop
	while (g_is_game_running) {
		// WSAAccept
		auto c_socket = WSAAccept(s_socket, reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == c_socket) err_display("WSAAccept");

		// Create SESSION
		for (int client_id = 0; client_id < MAX_CLIENT; ++client_id) {
			if (!g_clients[client_id]) {
				g_clients[client_id] = std::make_unique<SESSION>(client_id, c_socket, h_recv_callback);
				std::cout << "Client " << client_id << " Joined" << std::endl;
				break;
			}
		}
	}
}

//////////////////////////////////////////////////
// Play Game
void play_game() {
	while (g_is_game_running) {

	}
}

//////////////////////////////////////////////////
// Host WSARecv CALLBACK
void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	if ((0 != err) || (0 == num_bytes)) err_display("h_recv_callback");
	
	SESSION* o = reinterpret_cast<SESSION*>(p_over);
	o->recv_callback(num_bytes, p_over, g_q_lock, g_input_queue);
}