#pragma once

#include <WS2tcpip.h>
#include <stdio.h>

//////////////////////////////////////////////////
// Lobby
constexpr char S2C_LOBBY_LIST_PACKET = 1;
constexpr char S2C_JOIN_AVAILABILITY_PACKET = 2;
constexpr char S2C_HOST_ADDRESS_PACKET = 3;
constexpr char H2S_PLAYER_DISCONNECTION_PACKET = 4;
constexpr char C2S_CREATE_OR_JOIN_PACKET = 5;
constexpr char C2S_SELECTED_PAGE_PACKET = 6;
constexpr char C2S_SELECTED_LOBBY_PACKET = 7;

constexpr char LOBBY_COUNT_PER_PAGE = 25;
constexpr char MAX_LOBBY_NAME_LENGTH = 25;

struct LOBBY {
	unsigned short lobby_id;
	char lobby_name[MAX_LOBBY_NAME_LENGTH];
	char current_players;
};

//////////////////////////////////////////////////
// In-Game
constexpr char H2C_PLAYER_PACKET = 7;
constexpr char H2C_ITEM_PACKET = 8;
constexpr char H2C_SKILL_PACKET = 9;
constexpr char H2C_MONSTER_PACKET = 10;
constexpr char C2H_KEY_PACKET = 11;

constexpr char ANIMATION_STATE_IDLE = 1;
constexpr char ANIMATION_STATE_WALK = 2;
constexpr char ANIMATION_STATE_RUN = 3;
constexpr char ANIMATION_STATE_JUMP = 4;
constexpr char ANIMATION_STATE_ATTACK = 5;
constexpr char ANIMATION_STATE_HITTED = 6;

constexpr char ELEMENT_FIRE = 1;
constexpr char ELEMENT_ICE = 2;
constexpr char ELEMENT_WIND = 3;
constexpr char ELEMENT_EARTH = 4;

#pragma pack(push, 1)

//////////////////////////////////////////////////
// Lobby
struct sc_lobby_list_packet {
	unsigned char packet_size;
	char packet_type;
};

struct sc_join_availability_packet {
	unsigned char packet_size;
	char packet_type;
};

struct sc_host_address_packet {
	unsigned char packet_size;
	char packet_type;
};

struct hs_player_disconnection_packet {
	unsigned char packet_size;
	char packet_type;
};

struct cs_create_or_join_packet {
	unsigned char packet_size;
	char packet_type;
};

struct cs_selected_page_packet {
	unsigned char packet_size;
	char packet_type;
};

struct cs_selected_lobby_packet {
	unsigned char packet_size;
	char packet_type;
};

//////////////////////////////////////////////////
// In-Game

struct hc_player_packet {
	unsigned char packet_size;
	char packet_type;
	float x, y, z;
	float dx, dy, dz;
	char hp;
	char animation_state;
	char current_element;
};

struct hc_item_packet {
	unsigned char packet_size;
	char packet_type;
	char item_type;
	char item_quantity;
};

struct hc_skill_packet {
	unsigned char packet_size;
	char packet_type;
	float x, y, z;
	float dx, dy, dz;
	char skill_type;
	bool destroyed;
};

struct hc_monster_packet {
	unsigned char packet_size;
	char packet_type;
	float x, y, z;
	float dx, dy, dz;
	char hp;
	char monster_type;
	char animation_state;
};

struct ch_key_packet {
	unsigned char packet_size;
	char packet_type;
	char key;
};

#pragma pack(pop)

inline void err_display(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(char*)&lpMsgBuf, 0, NULL);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}