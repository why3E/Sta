#pragma once

#include <WS2tcpip.h>
#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include <array>
#include <mutex>
#include <queue>
#include <unordered_map>

//////////////////////////////////////////////////
// Lobby


//////////////////////////////////////////////////
// In-Game
constexpr char H2C_PLAYER_INFO_PACKET = 1;
constexpr char H2C_PLAYER_ENTER_PACKET = 2;
constexpr char H2C_PLAYER_LEAVE_PACKET = 3;

constexpr char H2C_INIT_MONSTER_PACKET = 11;
constexpr char H2C_MONSTER_MOVE_PACKET = 12;
constexpr char H2C_MONSTER_ATTACK_PACKET = 13;
constexpr char H2C_MONSTER_RESPAWN_PACKET = 14;

constexpr char H2C_PLAYER_MOVE_PACKET = 21;
constexpr char H2C_PLAYER_STOP_PACKET = 22;
constexpr char H2C_PLAYER_ROTATE_PACKET = 23;
constexpr char H2C_PLAYER_JUMP_PACKET = 24;
constexpr char H2C_PLAYER_READY_SKILL_PACKET = 25;
constexpr char H2C_PLAYER_CHANGE_ELEMENT_PACKET = 26;
constexpr char H2C_SKILL_VECTOR_PACKET = 27;
constexpr char H2C_SKILL_ROTATOR_PACKET = 28;
constexpr char H2C_COLLISION_PACKET = 29;
constexpr char H2C_SKILL_CREATE_PACKET = 30;

constexpr char C2H_PLAYER_MOVE_PACKET = 41;
constexpr char C2H_PLAYER_STOP_PACKET = 42;
constexpr char C2H_PLAYER_ROTATE_PACKET = 43;
constexpr char C2H_PLAYER_JUMP_PACKET = 44;
constexpr char C2H_PLAYER_READY_SKILL_PACKET = 45;
constexpr char C2H_PLAYER_CHANGE_ELEMENT_PACKET = 46;
constexpr char C2H_SKILL_VECTOR_PACKET = 47;
constexpr char C2H_SKILL_ROTATOR_PACKET = 48;
constexpr char C2H_COLLISION_PACKET = 49;
constexpr char C2H_SKILL_CREATE_PACKET = 50;





constexpr char ELEMENT_WIND = 1;
constexpr char ELEMENT_FIRE = 2;
constexpr char ELEMENT_STONE = 3;
constexpr char ELEMENT_ICE = 4;

constexpr char SKILL_WIND_CUTTER = 1;
constexpr char SKILL_WIND_TORNADO = 2;
constexpr char SKILL_WIND_WIND_TORNADO = 3;
constexpr char SKILL_WIND_FIRE_BOMB = 4;
constexpr char SKILL_WIND_ICE_BOMB = 5;
constexpr char SKILL_FIRE_BALL = 11;
constexpr char SKILL_FIRE_WALL = 12;
constexpr char SKILL_STONE_WAVE = 21;
constexpr char SKILL_STONE_SKILL = 22;
constexpr char SKILL_ICE_ARROW = 31;
constexpr char SKILL_ICE_WALL = 32;

constexpr char SKILL_SKILL_COLLISION = 1;
constexpr char SKILL_MONSTER_COLLISION = 2;
constexpr char SKILL_PLAYER_COLLISION = 3;

constexpr char MAX_CLIENTS = 4;
constexpr unsigned short MAX_MONSTERS_PER_PACKET = 5;

struct monster_init_info {
	unsigned short id;
	float hp;
	float x; float y; float z;
	float target_x; float target_y; float target_z;
};

#pragma pack(push, 1)

//////////////////////////////////////////////////
// Lobby


//////////////////////////////////////////////////
// In-Game 
struct hc_player_info_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	float yaw;
	float x, y, z;
	float vx, vy, vz;
	float hp;
	char element[2];
};

struct hc_player_leave_packet {
	unsigned char packet_size;
	char packet_type;
	char player_id;
};

struct hc_init_monster_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short count;
	monster_init_info monsters[0];
};

struct hc_monster_move_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned char id;
	float target_x; float target_y; float target_z;
};

struct hc_monster_attack_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned char id;
};

struct hc_monster_respawn_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned char id;
	float respawn_x; float respawn_y; float respawn_z;
};

struct player_move_packet {
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

struct player_ready_skill_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	bool is_left;
};

struct player_change_element_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	char element;
	bool is_left;
};

struct skill_vector_packet {
	unsigned char packet_size;
	char packet_type;
	char player_id;
	unsigned short skill_id;
	char skill_type;
	float skill_vx, skill_vy, skill_vz;
	bool is_left;
};

struct skill_rotator_packet {
	unsigned char packet_size;
	char packet_type;
	char player_id;
	unsigned short skill_id;
	char skill_type;
	float skill_x, skill_y, skill_z;
	float skill_pitch, skill_yaw, skill_roll;
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
	unsigned char old_skill_id;
	unsigned char new_skill_id;
	float new_skill_x, new_skill_y, new_skill_z;
};

#pragma pack(pop)

//////////////////////////////////////////////////
// Monster
enum class EventType {
	Target,
	Attack,
	Respawn
};

struct TargetEvent {
	int id;
	FVector target_location;
};

struct AttackEvent {
	int id;
};

struct RespawnEvent {
	int id;
	FVector respawn_location;
};

struct MonsterEvent {
	EventType event_type;

	union Data {
		TargetEvent target;
		AttackEvent attack;
		RespawnEvent respawn;

		Data() {}  
		~Data() {} 
	} data;

	MonsterEvent(const TargetEvent& e) {
		event_type = EventType::Target;
		new (&data.target) TargetEvent(e);
	}

	MonsterEvent(const AttackEvent& e) {
		event_type = EventType::Attack;
		new (&data.attack) AttackEvent(e);
	}

	MonsterEvent(const RespawnEvent& e) {
		event_type = EventType::Respawn;
		new (&data.attack) RespawnEvent(e);
	}

	~MonsterEvent() {
		switch (event_type) {
		case EventType::Target:
			data.target.~TargetEvent();
			break;

		case EventType::Attack:
			data.attack.~AttackEvent();
			break;

		case EventType::Respawn:
			data.respawn.~RespawnEvent();
			break;
		}
	}
};