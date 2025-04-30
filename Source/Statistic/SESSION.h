#pragma once

#include "Common.h"

extern SOCKET g_h_socket;

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
class SESSION {
public:
	EXP_OVER m_recv_over;

	SOCKET m_c_socket;

	long long m_id;
	float m_x, m_y, m_z;
	float m_dx, m_dy, m_dz;
	char m_hp;
	char m_animation_state;
	char m_current_element;

	LPWSAOVERLAPPED_COMPLETION_ROUTINE m_recv_callback;

public:
	SESSION();
	SESSION(long long id, SOCKET c_socket, LPWSAOVERLAPPED_COMPLETION_ROUTINE h_recv_callback);
	~SESSION();

	void do_recv();
	void do_send(void* buff);
	void recv_callback(DWORD num_bytes, LPWSAOVERLAPPED p_over, std::mutex& q_lock, std::queue<ch_key_packet>& input_queue);
};