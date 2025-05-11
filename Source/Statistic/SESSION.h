#pragma once

#include "Common.h"

class APlayerCharacter;
class AMySkillBase;
class ACharacter;

extern SOCKET g_h_socket;
extern std::atomic<bool> g_is_host;

extern std::array<APlayerCharacter*, MAX_CLIENTS> g_c_players;
extern std::unordered_map<unsigned short, AMySkillBase*> g_skills;
extern std::unordered_map<unsigned short, std::queue<unsigned short>> g_collisions;
extern std::unordered_map<unsigned short, ACharacter*> g_monsters;

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

//////////////////////////////////////////////////
// SESSION
enum STATE { ST_FREE, ST_INGAME };

class SESSION {
public:
	EXP_OVER m_recv_over;

	SOCKET m_c_socket;
	int m_remained;

	long long m_id;
	STATE m_state;

	LPWSAOVERLAPPED_COMPLETION_ROUTINE m_recv_callback;
	LPWSAOVERLAPPED_COMPLETION_ROUTINE m_send_callback;

public:
	SESSION();
	SESSION(long long id, SOCKET c_socket, LPWSAOVERLAPPED_COMPLETION_ROUTINE h_recv_callback, LPWSAOVERLAPPED_COMPLETION_ROUTINE h_send_callback);
	~SESSION();

	void do_recv();
	void do_send(void* buff);
};

//////////////////////////////////////////////////
// send_callback
inline void send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}