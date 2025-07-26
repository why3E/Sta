#include <WS2tcpip.h>
#include <array>
#include <atomic>
#include <iostream>
#include <concurrent_unordered_map.h>

#include "Common.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "msimg32.lib")

constexpr short SERVER_PORT = 5000;

SOCKET g_s_socket;
std::atomic<int> g_s_client_id = 0;

void g_process_packet(int id, char* packet);
void CALLBACK g_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
void CALLBACK g_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

//////////////////////////////////////////////////
// EXP_OVER
class EXP_OVER {
public:
	WSAOVERLAPPED m_over;
	char m_buffer[1024];
	WSABUF m_wsabuf[1];

public:
	EXP_OVER();
	~EXP_OVER();
};

EXP_OVER::EXP_OVER() {
	ZeroMemory(&m_over, sizeof(m_over));
	ZeroMemory(&m_buffer, sizeof(m_buffer));
	m_wsabuf[0].len = sizeof(m_buffer);
	m_wsabuf[0].buf = m_buffer;
}

EXP_OVER::~EXP_OVER() {

}

//////////////////////////////////////////////////
// SESSION
enum STATE { ST_FREE, ST_INGAME };

class LOBBY;

class SESSION {
public:
	EXP_OVER m_recv_over;

	long long m_id;
	SOCKET m_c_socket;

	int m_remained;

	char m_slot;

public:
	SESSION();
	SESSION(long long id, SOCKET c_socket);
	~SESSION();

	void do_recv();
	void do_send(void* buff);
};

SESSION::SESSION() {

}

SESSION::SESSION(long long id, SOCKET c_socket) {
	m_remained = 0;

	do_recv();
}

SESSION::~SESSION() {
	closesocket(m_c_socket);
}

void SESSION::do_recv() {
	DWORD recv_flag = 0;
	m_recv_over.m_wsabuf[0].buf = m_recv_over.m_buffer + m_remained;
	m_recv_over.m_wsabuf[0].len = sizeof(m_recv_over.m_buffer) - m_remained;
	auto ret = WSARecv(m_c_socket, m_recv_over.m_wsabuf, 1, NULL, &recv_flag, reinterpret_cast<WSAOVERLAPPED*>(&m_recv_over), g_recv_callback);
	if (ret == SOCKET_ERROR) {
		int err_code = WSAGetLastError();
		if (err_code != WSA_IO_PENDING) {
			std::wcout << L"[WSARecv Error] Socket: " << m_c_socket << L" Error Code: " << err_code << std::endl;
		}
	}
}

void SESSION::do_send(void* buff) {
	EXP_OVER* o = new EXP_OVER;
	unsigned char packet_size = reinterpret_cast<unsigned char*>(buff)[0];
	memcpy(o->m_buffer, buff, packet_size);
	o->m_wsabuf[0].len = packet_size;
	DWORD send_bytes;
	auto ret = WSASend(m_c_socket, o->m_wsabuf, 1, &send_bytes, 0, &(o->m_over), g_send_callback);
	if (ret == SOCKET_ERROR) {
		int err_code = WSAGetLastError();
		if (err_code != WSA_IO_PENDING) {
			std::wcout << L"[WSASendError] Socket: " << m_c_socket << L" Error Code: " << err_code << std::endl;
		}
	}
}

concurrency::concurrent_unordered_map<int, std::atomic<std::shared_ptr<SESSION>>> g_clients;

//////////////////////////////////////////////////
// LOBBY
constexpr char ELEMENT_WIND = 0;
constexpr char ELEMENT_FIRE = 1;
constexpr char ELEMENT_ICE = 2;
constexpr char ELEMENT_STONE = 3;

constexpr char INIT = 0;
constexpr char SUCCEED = 1;
constexpr char FAILED = 2;
constexpr char DISCONNECTED = 3;

class PLAYER {
public:
	int m_id;
	char m_left_element = ELEMENT_WIND;
	char m_right_element = ELEMENT_FIRE;

public:
	PLAYER(int id) : m_id(id) {}
};

class LOBBY {
public:
	bool m_is_game_started = false;
	std::array<PLAYER*, 4> m_players;

public:
	LOBBY() {
		for (int i = 0; i < 4; ++i) {
			m_players[i] = nullptr;
		}
	}

	void send_error_code(int id, char ec) {
		std::shared_ptr<SESSION> client = g_clients.at(id);
		if (nullptr == client) { return; }

		sc_error_code_packet p;
		p.packet_size = sizeof(sc_error_code_packet);
		p.packet_type = S2C_ERROR_CODE_PACKET;
		p.ec = ec;
		client->do_send(&p);
	}

	void send_add_player(int slot) {
		for (int i = 0; i < 4; ++i) {
			if (nullptr != m_players[i]) {
				if (i != slot) {
					std::shared_ptr<SESSION> client = g_clients.at(m_players[i]->m_id);
					if (nullptr == client) { continue; }

					sc_add_player_packet p;
					p.packet_size = sizeof(sc_add_player_packet);
					p.packet_type = S2C_ADD_PLAYER_PAKCET;
					p.slot = slot;
					client->do_send(&p);
				}
			}
		}

		std::shared_ptr<SESSION> client = g_clients.at(m_players[slot]->m_id);
		if (nullptr == client) { return; }

		for (int i = 0; i < 4; ++i) {
			if (nullptr != m_players[i]) {
				player_init_info player;
				player.slot = i;
				player.left_element = m_players[i]->m_left_element;
				player.right_element = m_players[i]->m_right_element;

				sc_init_lobby_packet p;
				p.packet_size = sizeof(sc_init_lobby_packet);
				p.packet_type = S2C_INIT_LOBBY_PAKCET;
				p.player = player;
				client->do_send(&p);
			}
		}
	}

	void send_remove_player(int slot) {
		m_players[slot] = nullptr;

		sc_remove_player_packet p;
		p.packet_size = sizeof(sc_remove_player_packet);
		p.packet_type = S2C_REMOVE_PLAYER_PAKCET;
		p.slot = slot;

		for (int i = 0; i < 4; ++i) {
			if (nullptr != m_players[i]) {
				if (i != slot) {
					std::shared_ptr<SESSION> client = g_clients.at(m_players[i]->m_id);
					if (nullptr == client) { continue; }

					client->do_send(&p);
				}
			}
		}
	}

	void send_change_element(int slot, bool is_left) {
		sc_change_element_packet p;
		p.packet_size = sizeof(sc_change_element_packet);
		p.packet_type = S2C_CHANGE_ELEMENT_PAKCET;
		p.slot = slot;
		p.is_left = is_left;
		p.element = is_left ? (m_players[slot]->m_left_element + 1) % 4 : (m_players[slot]->m_right_element + 1) % 4;

		is_left ? m_players[slot]->m_left_element = (m_players[slot]->m_left_element + 1) % 4 : m_players[slot]->m_right_element = (m_players[slot]->m_right_element + 1) % 4;

		for (int i = 0; i < 4; ++i) {
			if (nullptr != m_players[i]) {
				std::shared_ptr<SESSION> client = g_clients.at(m_players[i]->m_id);
				if (nullptr == client) { continue; }

				client->do_send(&p);
			}
		}
	}

	void send_start_game() {
		if (false == m_is_game_started) {
			sc_start_game_packet p;
			p.packet_size = sizeof(sc_start_game_packet);
			p.packet_type = S2C_START_GAME_PAKCET;

			std::shared_ptr<SESSION> client = g_clients.at(m_players[0]->m_id);
			if (nullptr == client) { return; }

			client->do_send(&p);

			m_is_game_started = true;
		} else {
			Sleep(1000);

			sc_start_game_packet p;
			p.packet_size = sizeof(sc_start_game_packet);
			p.packet_type = S2C_START_GAME_PAKCET;

			for (int i = 1; i < 4; ++i) {
				if (nullptr != m_players[i]) {
					std::shared_ptr<SESSION> client = g_clients.at(m_players[i]->m_id);
					if (nullptr == client) { continue; }

					client->do_send(&p);
				}
			}
		}
	}

	void add_player(PLAYER*& player) {
		int slot = -1;

		for (int i = 0; i < 4; ++i) {
			if (nullptr == m_players[i]) {
				m_players[i] = player;
				slot = i;
				break;
			}
		}

		if (0 == slot) {
			std::shared_ptr<SESSION> client = g_clients.at(player->m_id);
			if (nullptr == client) { return; }

			client->m_slot = slot;

			send_error_code(player->m_id, INIT);
			send_add_player(slot);
		} else if (0 < slot) {
			std::shared_ptr<SESSION> client = g_clients.at(player->m_id);
			if (nullptr == client) { return; }

			client->m_slot = slot;

			send_error_code(player->m_id, SUCCEED);
			send_add_player(slot);
		} else {
			send_error_code(player->m_id, FAILED);
		}
	}
};

concurrency::concurrent_unordered_map<int, std::atomic<std::shared_ptr<LOBBY>>> g_id_to_lobby;
concurrency::concurrent_unordered_map<std::string, std::atomic<std::shared_ptr<LOBBY>>> g_ip_to_lobby;

//////////////////////////////////////////////////
// main
int main() {
	// WSAStartup
	WSADATA WSAData;
	auto ret = WSAStartup(MAKEWORD(2, 2), &WSAData);

	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	u_long noblock = 1;
	ioctlsocket(g_s_socket, FIONBIO, &noblock);

	// Bind & Listen
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	ret = bind(g_s_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN));
	ret = listen(g_s_socket, SOMAXCONN);
	
	// WSAAccept
	INT addr_size = sizeof(SOCKADDR_IN);

	while (true) {
		auto c_socket = WSAAccept(g_s_socket, reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);

		if (INVALID_SOCKET == c_socket) {
			if (WSAEWOULDBLOCK == WSAGetLastError()) {
				SleepEx(1, TRUE);
				continue;
			}
		}

		int client_id = g_s_client_id++;
		std::shared_ptr<SESSION> p = std::make_shared<SESSION>();
		p->m_id = client_id;
		p->m_c_socket = c_socket;
		g_clients.insert(std::make_pair(client_id, p));
		p->do_recv();
	}

	closesocket(g_s_socket);

	WSACleanup();
}

void g_process_packet(int id, char* packet) {
	char packet_type = packet[1];
	std::cout << "Received Packet " << packet_type << std::endl;

	switch (packet_type) {
	case C2S_INIT_LOBBY_PACKET: {
		std::shared_ptr<SESSION> client = g_clients.at(id);
		if (nullptr == client) { return; }

		cs_init_lobby_packet* p = reinterpret_cast<cs_init_lobby_packet*>(packet);
		std::string ip_address(p->ip_address);
		auto lobby = g_ip_to_lobby.find(ip_address);
		if (lobby == g_ip_to_lobby.end()) {
			auto new_lobby = std::make_shared<LOBBY>();
			PLAYER* new_player = new PLAYER(id);
			new_lobby->add_player(new_player);
			g_ip_to_lobby.insert(std::make_pair(ip_address, new_lobby));
			g_id_to_lobby.insert(std::make_pair(id, new_lobby));
		} else {
			auto old_lobby = lobby->second.load();
			PLAYER* newPlayer = new PLAYER(id);  
			old_lobby->add_player(newPlayer);
			g_id_to_lobby.insert(std::make_pair(id, old_lobby));
		}
		break; }

	case C2S_REMOVE_PLAYER_PAKCET: {
		std::shared_ptr<SESSION> client = g_clients.at(id);
		if (nullptr == client) { return; }

		auto lobby = g_id_to_lobby.find(id);
		lobby->second.load()->send_remove_player(client->m_slot);
		break; }

	case C2S_CHANGE_ELEMENT_PAKCET: {
		std::shared_ptr<SESSION> client = g_clients.at(id);
		if (nullptr == client) { return; }

		cs_change_element_packet* p = reinterpret_cast<cs_change_element_packet*>(packet);

		auto lobby = g_id_to_lobby.find(id);
		lobby->second.load()->send_change_element(client->m_slot, p->is_left);
		break; }

	case C2S_START_GAME_PAKCET: {
		std::shared_ptr<SESSION> client = g_clients.at(id);
		if (nullptr == client) { return; }

		auto lobby = g_id_to_lobby.find(id);
		lobby->second.load()->send_start_game();
		break;
	}

	default:
		break;
	}
}

void g_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	SESSION* o = reinterpret_cast<SESSION*>(p_over);

	if (!o) { return; }

	if ((0 != err) || (0 == num_bytes)) {
		return;
	}

	// Process Packet
	char* p = o->m_recv_over.m_buffer;
	unsigned char packet_size = p[0];
	int remained = o->m_remained + num_bytes;

	while (packet_size <= remained) {
		g_process_packet(o->m_id, p);
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

void g_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}
