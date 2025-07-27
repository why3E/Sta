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
#include <random>

//////////////////////////////////////////////////
// Lobby
constexpr char S2C_INIT_LOBBY_PAKCET = 0;
constexpr char S2C_ADD_PLAYER_PAKCET = 1;
constexpr char S2C_ERROR_CODE_PACKET = 2;
constexpr char S2C_REMOVE_PLAYER_PAKCET = 3;
constexpr char S2C_CHANGE_ELEMENT_PAKCET = 4;
constexpr char S2C_START_GAME_PAKCET = 5;

constexpr char C2S_INIT_LOBBY_PACKET = 20;
constexpr char C2S_REMOVE_PLAYER_PAKCET = 21;
constexpr char C2S_CHANGE_ELEMENT_PAKCET = 22;
constexpr char C2S_START_GAME_PAKCET = 23;

constexpr char INIT = 0;
constexpr char SUCCEED = 1;
constexpr char FAILED = 2;
constexpr char DISCONNECTED = 3;

//////////////////////////////////////////////////
// In-Game
constexpr char H2C_TIME_OFFSET_PACKET = 0;

constexpr char H2C_PLAYER_INFO_PACKET = 1;
constexpr char H2C_PLAYER_ENTER_PACKET = 2;
constexpr char H2C_PLAYER_LEAVE_PACKET = 3;

constexpr char H2C_INIT_MONSTER_PACKET = 11;
constexpr char H2C_MONSTER_MOVE_PACKET = 12;
constexpr char H2C_MONSTER_ATTACK_PACKET = 13;
constexpr char H2C_MONSTER_SKILL_PACKET = 14;
constexpr char H2C_MONSTER_HEAL_PACKET = 15;
constexpr char H2C_MONSTER_DAMAGED_PACKET = 16;
constexpr char H2C_MONSTER_DIE_PACKET = 17;
constexpr char H2C_MONSTER_RESPAWN_PACKET = 18;

constexpr char H2C_PLAYER_MOVE_PACKET = 21;
constexpr char H2C_PLAYER_STOP_PACKET = 22;
constexpr char H2C_PLAYER_ROTATE_PACKET = 23;
constexpr char H2C_PLAYER_JUMP_PACKET = 24;
constexpr char H2C_PLAYER_TELEPORT_PACKET = 25;
constexpr char H2C_PLAYER_READY_SKILL_PACKET = 26;
constexpr char H2C_PLAYER_CHANGE_ELEMENT_PACKET = 27;
constexpr char H2C_SKILL_VECTOR_PACKET = 29;
constexpr char H2C_SKILL_ROTATOR_PACKET = 30;
constexpr char H2C_GIMMICK_START_PACKET = 31;
constexpr char H2C_GIMMICK_END_PACKET = 32;

constexpr char H2C_SKILL_SKILL_COLLISION_PACKET = 60;
constexpr char H2C_SKILL_MONSTER_COLLISION_PACKET = 61;
constexpr char H2C_SKILL_PLAYER_COLLISION_PACKET = 62;
constexpr char H2C_SKILL_OBJECT_COLLISION_PACKET = 63;
constexpr char H2C_MONSTER_SKILL_COLLISION_PACKET = 64;
constexpr char H2C_PLAYER_SKILL_COLLISION_PACKET = 65;
constexpr char H2C_OBJECT_SKILL_COLLISION_PACKET = 66;

constexpr char H2C_SKILL_CREATE_PACKET = 70;

constexpr char C2H_PLAYER_MOVE_PACKET = 41;
constexpr char C2H_PLAYER_STOP_PACKET = 42;
constexpr char C2H_PLAYER_ROTATE_PACKET = 43;
constexpr char C2H_PLAYER_JUMP_PACKET = 44;
constexpr char C2H_PLAYER_TELEPORT_PACKET = 45;
constexpr char C2H_PLAYER_READY_SKILL_PACKET = 46;
constexpr char C2H_PLAYER_CHANGE_ELEMENT_PACKET = 47;
constexpr char C2H_SKILL_VECTOR_PACKET = 49;
constexpr char C2H_SKILL_ROTATOR_PACKET = 50;
constexpr char C2H_GIMMICK_START_PACKET = 51;
constexpr char C2H_GIMMICK_END_PACKET = 52;

constexpr int SECTOR_WIDTH = 10000;
constexpr int SECTOR_HEIGHT = 10000;
constexpr int SECTOR_ROWS = 100'000 / SECTOR_WIDTH;
constexpr int SECTOR_COLS = 100'000 / SECTOR_HEIGHT;




constexpr char ELEMENT_WIND = 0;
constexpr char ELEMENT_FIRE = 1;
constexpr char ELEMENT_ICE = 2;
constexpr char ELEMENT_STONE = 3;

constexpr char INVALID_SKILL_ID = -1;
constexpr char SKILL_WIND_CUTTER = 1;
constexpr char SKILL_WIND_TORNADO = 2;
constexpr char SKILL_WIND_LASER = 3;
constexpr char SKILL_WIND_WIND_TORNADO = 4;
constexpr char SKILL_WIND_FIRE_BOMB = 5;
constexpr char SKILL_WIND_ICE_BOMB = 6;
constexpr char SKILL_FIRE_BALL = 11;
constexpr char SKILL_FIRE_WALL = 12;
constexpr char SKILL_STONE_WAVE = 21;
constexpr char SKILL_STONE_SKILL = 22;
constexpr char SKILL_ICE_ARROW = 31;
constexpr char SKILL_ICE_WALL = 32;
constexpr char SKILL_WIND_BOOM = 33;

constexpr char MAX_CLIENTS = 4;
constexpr unsigned short MAX_MONSTERS_PER_PACKET = 5;

constexpr int32 STATUE_ID_START = 100;
constexpr unsigned short MONSTER_ID_START = 1000;
constexpr unsigned short INVALID_OBJECT_ID = 65535;

//////////////////////////////////////////////////
// Monster
enum class MonsterEventType {
	Target,
	Attack,
	Skill,
	Heal,
	Damaged,
	Die,
	Respawn
};

enum class MonsterType {
	Cactus,
	Slime,
	Swarm,
	MidBoss,
	Boss,
	Unknown
};

enum class AttackType {
	Melee,
	Range,
	WindCutter,
	WindLaser,
	StoneWave,
	WindTornado,
	StoneSkill,
	IceArrow,
	IceWall,
	FireBall,
	WindBoom
};

struct monster_init_info {
	char type;
	unsigned short id;
	float hp;
	float x; float y; float z;
	float target_x; float target_y; float target_z;
};

struct TargetEvent {
	unsigned short id;
	FVector target_location;
};

struct AttackEvent {
	unsigned short id;
	FVector location;
	AttackType attack_type;
};

struct SkillEvent {
	unsigned short id;
	FVector location;
	AttackType skill_type;
	FVector skill_location;
};

struct HealEvent {
	unsigned short id;
	float heal_amount;
};

struct DamagedEvent {
	unsigned short id;
};

struct DieEvent {
	unsigned short id;
};

struct RespawnEvent {
	unsigned short id;
	FVector respawn_location;
};

struct MonsterEvent {
	MonsterEventType monster_event_type;

	union Data {
		TargetEvent target;
		AttackEvent attack;
		SkillEvent skill;
		HealEvent heal;
		DamagedEvent damaged;
		DieEvent die;
		RespawnEvent respawn;

		Data() {}
		~Data() {}
	} data;

	MonsterEvent(const TargetEvent& e) {
		monster_event_type = MonsterEventType::Target;
		new (&data.target) TargetEvent(e);
	}

	MonsterEvent(const AttackEvent& e) {
		monster_event_type = MonsterEventType::Attack;
		new (&data.attack) AttackEvent(e);
	}

	MonsterEvent(const SkillEvent& e) {
		monster_event_type = MonsterEventType::Skill;
		new (&data.skill) SkillEvent(e);
	}

	MonsterEvent(const HealEvent& e) {
		monster_event_type = MonsterEventType::Heal;
		new (&data.heal) HealEvent(e);
	}

	MonsterEvent(const DamagedEvent& e) {
		monster_event_type = MonsterEventType::Damaged;
		new (&data.damaged) DamagedEvent(e);
	}

	MonsterEvent(const DieEvent& e) {
		monster_event_type = MonsterEventType::Die;
		new (&data.damaged) DieEvent(e);
	}

	MonsterEvent(const RespawnEvent& e) {
		monster_event_type = MonsterEventType::Respawn;
		new (&data.attack) RespawnEvent(e);
	}

	~MonsterEvent() {
		switch (monster_event_type) {
		case MonsterEventType::Target:
			data.target.~TargetEvent();
			break;

		case MonsterEventType::Attack:
			data.attack.~AttackEvent();
			break;

		case MonsterEventType::Skill:
			data.skill.~SkillEvent();
			break;

		case MonsterEventType::Heal:
			data.heal.~HealEvent();
			break;

		case MonsterEventType::Damaged:
			data.damaged.~DamagedEvent();
			break;

		case MonsterEventType::Die:
			data.die.~DieEvent();
			break;

		case MonsterEventType::Respawn:
			data.respawn.~RespawnEvent();
			break;
		}
	}
};

//////////////////////////////////////////////////
// Collision
enum class CollisionType {
	SkillSkill,
	SkillMonster,
	SkillPlayer,
	SkillObject,
	MonsterSkill,
	PlayerSkill,
	ObjectSkill
};

struct SkillSkillEvent {
	unsigned short skill_id;
	char skill_type;
};

struct SkillMonsterEvent {
	unsigned short skill_id;
};

struct SkillPlayerEvent {
	unsigned short skill_id;
	char player_id;
};

struct SkillObjectEvent {
	unsigned short skill_id;
};

struct MonsterSkillEvent {
	unsigned short monster_id;
	char skill_type;
	FVector skill_location;
};

struct PlayerSkillEvent {
	char player_id;
	char skill_type;
};

struct ObjectSkillEvent {
	unsigned short object_id;
};

struct CollisionEvent {
	CollisionType collision_type;
	bool collision_start = true;

	union Data {
		SkillSkillEvent skill_skill;
		SkillMonsterEvent skill_monster;
		SkillPlayerEvent skill_player;
		SkillObjectEvent skill_object;
		MonsterSkillEvent monster_skill;
		PlayerSkillEvent player_skill;
		ObjectSkillEvent object_skill;

		Data() {}
		~Data() {}
	} data;

	CollisionEvent(const SkillSkillEvent& e) {
		collision_type = CollisionType::SkillSkill;
		new (&data.skill_skill) SkillSkillEvent(e);
	}

	CollisionEvent(const SkillMonsterEvent& e) {
		collision_type = CollisionType::SkillMonster;
		new (&data.skill_monster) SkillMonsterEvent(e);
	}

	CollisionEvent(const SkillPlayerEvent& e) {
		collision_type = CollisionType::SkillPlayer;
		new (&data.skill_player) SkillPlayerEvent(e);
	}

	CollisionEvent(const SkillObjectEvent& e) {
		collision_type = CollisionType::SkillObject;
		new (&data.skill_object) SkillObjectEvent(e);
	}

	CollisionEvent(const MonsterSkillEvent& e) {
		collision_type = CollisionType::MonsterSkill;
		new (&data.monster_skill) MonsterSkillEvent(e);
	}

	CollisionEvent(const PlayerSkillEvent& e) {
		collision_type = CollisionType::PlayerSkill;
		new (&data.player_skill) PlayerSkillEvent(e);
	}

	CollisionEvent(const ObjectSkillEvent& e) {
		collision_type = CollisionType::ObjectSkill;
		new (&data.object_skill) ObjectSkillEvent(e);
	}

	~CollisionEvent() {
		switch (collision_type) {
		case CollisionType::SkillSkill:
			data.skill_skill.~SkillSkillEvent();
			break;

		case CollisionType::SkillMonster:
			data.skill_monster.~SkillMonsterEvent();
			break;

		case CollisionType::SkillPlayer:
			data.skill_player.~SkillPlayerEvent();
			break;

		case CollisionType::MonsterSkill:
			data.monster_skill.~MonsterSkillEvent();
			break;

		case CollisionType::PlayerSkill:
			data.player_skill.~PlayerSkillEvent();
			break;

		case CollisionType::ObjectSkill:
			data.object_skill.~ObjectSkillEvent();
			break;
		}
	}
};

//////////////////////////////////////////////////
// Etc
enum class EventType {
	SkillCreate
};

struct SkillCreateEvent {
	unsigned short skill_id;
	char skill_type;
	FVector skill_location;
};

struct Event {
	EventType event_type;

	union Data {
		SkillCreateEvent skill_create;

		Data() {}
		~Data() {}
	} data;

	Event(const SkillCreateEvent& e) {
		event_type = EventType::SkillCreate;
		new (&data.skill_create) SkillCreateEvent(e);
	}

	~Event() {
		switch (event_type) {
		case EventType::SkillCreate:
			data.skill_create.~SkillCreateEvent();
			break;
		}
	}
};

#pragma pack(push, 1)

//////////////////////////////////////////////////
// Lobby
struct player_init_info {
	char slot;
	char left_element;
	char right_element;
};

struct sc_error_code_packet {
	unsigned char packet_size;
	char packet_type;
	char ec;
};

struct sc_init_lobby_packet {
	unsigned char packet_size;
	char packet_type;
	player_init_info player;
};

struct sc_add_player_packet {
	unsigned char packet_size;
	char packet_type;
	char slot;
};

struct sc_remove_player_packet {
	unsigned char packet_size;
	char packet_type;
	char slot;
};

struct sc_change_element_packet {
	unsigned char packet_size;
	char packet_type;
	char slot;
	bool is_left;
	char element;
};

struct sc_start_game_packet {
	unsigned char packet_size;
	char packet_type;
	char num_of_players;
	char ip_address[16];
};

struct cs_init_lobby_packet {
	unsigned char packet_size;
	char packet_type;
	char ip_address[16];
};

struct cs_remove_player_packet {
	unsigned char packet_size;
	char packet_type;
};

struct cs_change_element_packet {
	unsigned char packet_size;
	char packet_type;
	bool is_left;
};

struct cs_start_game_packet {
	unsigned char packet_size;
	char packet_type;
};

//////////////////////////////////////////////////
// In-Game 
struct hc_time_offset_packet {
	unsigned char packet_size;
	char packet_type;
	float time;
};

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
	unsigned short id;
	float target_x; float target_y; float target_z;
};

struct hc_monster_attack_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short id;
	float x; float y; float z;
	char attack_type;
};

struct hc_monster_skill_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short id;
	float x; float y; float z;
	unsigned short skill_id;
	char skill_type;
	float skill_x; float skill_y; float skill_z;
};

struct hc_monster_heal_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short id;
	float heal_amount;
};

struct hc_monster_damaged_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short id;
};

struct hc_monster_die_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short id;
};

struct hc_monster_respawn_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short id;
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

struct player_teleport_packet {
	unsigned char packet_size;
	char packet_type;
	char id;
	float x, y, z;
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
	float time;
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
	float time;
};

struct skill_skill_collision_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short skill_id;
	char skill_type;
};

struct skill_monster_collision_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short skill_id;
};

struct skill_player_collision_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short skill_id;
	char player_id;
	bool collision_start;
};

struct skill_object_collision_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short skill_id;
};

struct monster_skill_collision_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short monster_id;
	char skill_type;
	float skill_x, skill_y, skill_z;
};

struct player_skill_collision_packet {
	unsigned char packet_size;
	char packet_type;
	char player_id;
	char skill_type;
	bool collision_start;
};

struct object_skill_collision_packet {
	unsigned char packet_size;
	char packet_type;
	char object_id;
};

struct skill_create_packet {
	unsigned char packet_size;
	char packet_type;
	char skill_type;
	unsigned char old_skill_id;
	unsigned char new_skill_id;
	float new_skill_x, new_skill_y, new_skill_z;
};

struct gimmick_start_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short object_id;
};

struct gimmick_end_packet {
	unsigned char packet_size;
	char packet_type;
	unsigned short object_id;
	bool succeed;
};

#pragma pack(pop)