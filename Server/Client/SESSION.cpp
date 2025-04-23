#include "SESSION.h"

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

SESSION::SESSION(long long client_id, SOCKET c_socket, LPWSAOVERLAPPED_COMPLETION_ROUTINE h_recv_callback) : m_c_socket(c_socket), m_client_id(client_id) {
	m_recv_callback = h_recv_callback;

	do_recv();
}

SESSION::~SESSION() {
	closesocket(m_c_socket);
}

void SESSION::do_recv() {
	DWORD recv_flag = 0;
	auto ret = WSARecv(m_c_socket, m_recv_over.m_wsabuf, 1, NULL, &recv_flag, &m_recv_over.m_over, m_recv_callback);
	if (ret == SOCKET_ERROR) if (WSAGetLastError() != WSA_IO_PENDING) err_display("WSARecv");
}

void SESSION::do_send() {

}

void SESSION::recv_callback(DWORD num_bytes, LPWSAOVERLAPPED p_over, std::mutex& q_lock, std::queue<ch_key_packet>& input_queue) {
	// Enqueue
	{
		std::lock_guard<std::mutex> lock(q_lock);
		/* Enqueue */
	}

	do_recv();
}
