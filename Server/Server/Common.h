#pragma once

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

#pragma pack(push, 1)

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

#pragma pack(pop)