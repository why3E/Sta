#pragma once

#include <WS2tcpip.h>
#include <iostream>
#include <mutex>
#include <queue>
#include <unordered_map>

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
constexpr char H2C_PLAYER_STOP_PACKET = 22;
constexpr char H2C_PLAYER_ROTATE_PACKET = 23;
constexpr char H2C_PLAYER_JUMP_PACKET = 24;
constexpr char H2C_PLAYER_SKILL_VECTOR_PACKET = 25;
constexpr char H2C_PLAYER_SKILL_ROTATOR_PACKET = 26;
constexpr char H2C_PLAYER_CHANGE_ELEMENT_PACKET = 27;
constexpr char H2C_COLLISION_PACKET = 28;
constexpr char H2C_SKILL_CREATE_PACKET = 29;

constexpr char C2H_PLAYER_VECTOR_PACKET = 41;
constexpr char C2H_PLAYER_STOP_PACKET = 42;
constexpr char C2H_PLAYER_ROTATE_PACKET = 43;
constexpr char C2H_PLAYER_JUMP_PACKET = 44;
constexpr char C2H_PLAYER_SKILL_VECTOR_PACKET = 45;
constexpr char C2H_PLAYER_SKILL_ROTATOR_PACKET = 46;
constexpr char C2H_PLAYER_CHANGE_ELEMENT_PACKET = 47;
constexpr char C2H_COLLISION_PACKET = 48;
constexpr char C2H_SKILL_CREATE_PACKET = 49;



constexpr char ELEMENT_WIND = 1;
constexpr char ELEMENT_FIRE = 2;

constexpr char SKILL_WIND_CUTTER = 1;
constexpr char SKILL_WIND_TORNADO = 2;
constexpr char SKILL_WIND_WIND_TORNADO = 3;
constexpr char SKILL_WIND_FIRE_TORNADO = 4;
constexpr char SKILL_WIND_FIRE_BOMB = 5;
constexpr char SKILL_FIRE_BALL = 6;
constexpr char SKILL_FIRE_WALL = 7;

constexpr char SKILL_SKILL_COLLISION = 1;

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

struct player_vector_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	float x, y, z;
	float vx, vy, vz;
};

struct player_stop_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	float x, y, z;
};

struct player_rotate_packet {
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

struct hc_player_skill_vector_packet {
	unsigned char packet_size;
	char packet_type;
	char player_id;
	unsigned short skill_id;
	char skill_type;
	float x, y, z;
	bool is_left;
};

struct ch_player_skill_vector_packet {
	unsigned char packet_size;
	char packet_type;
	char player_id;
	char skill_type;
	float x, y, z;
	bool is_left;
};

struct hc_player_skill_rotator_packet {
	unsigned char packet_size;
	char packet_type;
	char player_id;
	unsigned short skill_id;
	char skill_type;
	float x, y, z;
	float pitch, yaw, roll;
	bool is_left;
};

struct ch_player_skill_rotator_packet {
	unsigned char packet_size;
	char packet_type;
	char player_id;
	char skill_type;
	float x, y, z;
	float pitch, yaw, roll;
	bool is_left;
};

struct player_change_element_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	char element_type;
	bool is_left;
};

struct collision_packet {
	unsigned char packet_size;
	char packet_type;
	char collision_type;
	unsigned char attacker_id;
	unsigned char victim_id;
};

struct skill_create_packet {
	unsigned char packet_size;
	char packet_type;
	char skill_type;
	unsigned char skill_id;
	float x, y, z;
};

#pragma pack(pop)