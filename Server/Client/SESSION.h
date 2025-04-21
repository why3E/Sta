#pragma once

#include "..\Common.h"

class EXP_OVER;
class SESSION;

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
private:
	EXP_OVER m_recv_over;

	SOCKET m_c_socket;
	long long m_client_id;

	LPWSAOVERLAPPED_COMPLETION_ROUTINE m_recv_callback;

public:
	SESSION();
	SESSION(long long client_id, SOCKET c_socket, LPWSAOVERLAPPED_COMPLETION_ROUTINE h_recv_callback);
	~SESSION();

	void do_recv();
	void do_send();
	void recv_callback(DWORD num_bytes, LPWSAOVERLAPPED p_over, std::mutex& q_lock, std::queue<ch_key_packet>& input_queue);
};