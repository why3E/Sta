#pragma once

#include <WS2tcpip.h>
#include <iostream>
#include <mutex>
#include <queue>

//////////////////////////////////////////////////
// Lobby
constexpr char S2C_LOBBY_LIST_PACKET = 1;
constexpr char S2C_JOIN_AVAILABILITY_PACKET = 2;
constexpr char S2C_HOST_ADDRESS_PACKET = 3;
constexpr char H2S_PLAYER_DISCONNECTION_PACKET = 4;
constexpr char C2S_CREATE_OR_JOIN_PACKET = 5;
constexpr char C2S_SELECTED_PAGE_PACKET = 6;
constexpr char C2S_SELECTED_LOBBY_PACKET = 7;

//////////////////////////////////////////////////
// In-Game
constexpr char H2C_PLAYER_INFO_PACKET = 1;
constexpr char H2C_PLAYER_ENTER_PACKET = 2;
constexpr char H2C_PLAYER_LEAVE_PACKET = 3;

constexpr char H2C_PLAYER_PACKET = 11;
constexpr char H2C_ITEM_PACKET = 12;
constexpr char H2C_SKILL_PACKET = 13;
constexpr char H2C_MONSTER_PACKET = 14;

constexpr char H2C_PLAYER_VECTOR_PACKET = 21;
constexpr char H2C_PLAYER_STOPPED_PACKET = 22;
constexpr char H2C_PLAYER_DIRECTION_PACKET = 23;
constexpr char H2C_PLAYER_JUMP_START_PACKET = 24;
constexpr char H2C_PLAYER_SKILL_PACKET = 25;

constexpr char C2H_PLAYER_VECTOR_PACKET = 41;
constexpr char C2H_PLAYER_STOPPED_PACKET = 42;
constexpr char C2H_PLAYER_DIRECTION_PACKET = 43;
constexpr char C2H_PLAYER_JUMP_START_PACKET = 44;
constexpr char C2H_PLAYER_SKILL_PACKET = 45;



constexpr char ANIMATION_STATE_IDLE = 1;

constexpr char SKILL_WIND_CUTTER = 1;
constexpr char SKILL_WIND_TORNADO = 2;
constexpr char SKILL_FIRE_BALL = 3;
constexpr char SKILL_FIRE_WALL = 4;

constexpr char MAX_CLIENTS = 4;

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
struct hc_player_info_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	float yaw;
	float vx, vy, vz;
	char hp;
	char animation_state;
	char current_element;
};

struct hc_player_leave_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
};

struct hc_item_packet {
	unsigned char packet_size;
	char packet_type;
};

struct hc_skill_packet {
	unsigned char packet_size;
	char packet_type;
};

struct hc_monster_packet {
	unsigned char packet_size;
	char packet_type;
};

struct player_vector_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	float x, y, z;
	float vx, vy, vz;
};

struct player_stopped_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	float x, y, z;
};

struct player_rotation_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	float yaw;
};

struct player_jump_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
};

struct player_skill_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	char skill_type;
	float x, y, z;
};

#pragma pack(pop)