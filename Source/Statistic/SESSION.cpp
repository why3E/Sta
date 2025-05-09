#include "SESSION.h"

SOCKET g_h_socket;

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

SESSION::SESSION(long long id, SOCKET c_socket, 
	LPWSAOVERLAPPED_COMPLETION_ROUTINE h_recv_callback, LPWSAOVERLAPPED_COMPLETION_ROUTINE h_send_callback) : m_c_socket(c_socket), m_id(id) {

	m_remained = 0;

	m_yaw = 0.0f;
	m_velocity = FVector(0.0f, 0.0f, 0.0f);
	m_hp = 100;
	m_current_element = ELEMENT_WIND;

	m_recv_callback = h_recv_callback;
	m_send_callback = h_send_callback; 

	do_recv();
}

SESSION::~SESSION() {
	closesocket(m_c_socket);
}

void SESSION::do_recv() {
	DWORD recv_flag = 0;
	m_recv_over.m_wsabuf[0].buf = m_recv_over.m_buffer + m_remained;
	m_recv_over.m_wsabuf[0].len = sizeof(m_recv_over.m_buffer) - m_remained;
	auto ret = WSARecv(m_c_socket, m_recv_over.m_wsabuf, 1, NULL, &recv_flag, reinterpret_cast<WSAOVERLAPPED*>(&m_recv_over), m_recv_callback);

	if (ret == SOCKET_ERROR) {
		DWORD err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			UE_LOG(LogTemp, Error, TEXT("WSARecv failed: %d"), err);
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("do_recv() called on session %d"), m_id);
}

void SESSION::do_send(void* buff) {
	EXP_OVER* o = new EXP_OVER;
	unsigned char packet_size = reinterpret_cast<unsigned char*>(buff)[0];
	memcpy(o->m_buffer, buff, packet_size);
	o->m_wsabuf[0].len = packet_size;
	DWORD send_bytes;
	auto ret = WSASend(m_c_socket, o->m_wsabuf, 1, &send_bytes, 0, &(o->m_over), m_send_callback);

	if (ret == SOCKET_ERROR) {
		DWORD err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			UE_LOG(LogTemp, Error, TEXT("WSASend failed: %d"), err);
			delete o;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("do_send() called on session %d"), m_id);
}