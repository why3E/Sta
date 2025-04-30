#include "SESSION.h"

SOCKET g_h_socket;
char g_id;
float g_x, g_y, g_z;
float g_dx, g_dy, g_dz;
char g_hp;
char g_animation_state;
char g_current_element;

//////////////////////////////////////////////////
// EXP_OVER
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
SESSION::SESSION() {

}

SESSION::SESSION(long long id, SOCKET c_socket, LPWSAOVERLAPPED_COMPLETION_ROUTINE h_recv_callback) : m_c_socket(c_socket), m_id(id) {
	m_recv_callback = h_recv_callback;

	do_recv();
}

SESSION::~SESSION() {
	closesocket(m_c_socket);
}

void SESSION::do_recv() {
	DWORD recv_flag = 0;
	auto ret = WSARecv(m_c_socket, m_recv_over.m_wsabuf, 1, NULL, &recv_flag, &m_recv_over.m_over, m_recv_callback);
}

void SESSION::do_send(void* buff) {
	EXP_OVER* o = new EXP_OVER;
	unsigned char packet_size = reinterpret_cast<unsigned char*>(buff)[0];
	memcpy(o->m_buffer, buff, packet_size);
	o->m_wsabuf[0].len = packet_size;
	DWORD send_bytes;
	WSASend(m_c_socket, o->m_wsabuf, 1, &send_bytes, 0, &(o->m_over), NULL);
}

void SESSION::recv_callback(DWORD num_bytes, LPWSAOVERLAPPED p_over, std::mutex& q_lock, std::queue<player_vector_packet>& input_queue) {
	// Enqueue
	{
		std::lock_guard<std::mutex> lock(q_lock);
		/* 
			Enqueue 
		*/
	}

	do_recv();
}
