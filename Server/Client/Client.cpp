#include <WS2tcpip.h>
#include <thread>
#include "..\Common.h"

#pragma comment (lib, "WS2_32.LIB")

constexpr bool HOST = true;
constexpr short HOST_PORT = 3000;
constexpr char HOST_ADDRESS[] = "127.0.0.1";

void server_thread();
void game_loop();

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
	if (HOST)
		std::thread s_thread(server_thread);

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT);
	inet_pton(AF_INET, HOST_ADDRESS, &addr.sin_addr);
	ret = WSAConnect(h_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == ret) err_display("WSAConnect");

	game_loop();
}

void server_thread() {

}

void game_loop() {

}