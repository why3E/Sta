// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"
#include "MySkillBase.h"
#include "MyWindCutter.h"
#include "MyWindSkill.h"
#include "AIController.h"
#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Blueprint/UserWidget.h"

#include "SESSION.h"

#include <atomic>
#include <chrono>
#include <thread>

constexpr short HOST_PORT = 3000;
FString HOST_ADDRESS = TEXT("127.0.0.1");

//////////////////////////////////////////////////
// Server
SOCKET g_s_socket;
std::thread g_s_thread;
std::atomic<bool> g_s_running;
std::atomic<unsigned short> g_s_skill_id;
std::atomic<unsigned short> g_s_monster_id;

std::array<std::unique_ptr<SESSION>, MAX_CLIENTS> g_s_clients;

void server_thread();
void accept_thread();
void h_process_packet(char* p);
extern void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
extern void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

//////////////////////////////////////////////////
// Client
EXP_OVER g_recv_over;
int g_remained;

void c_process_packet(char* p);
void CALLBACK c_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);
void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags);

//////////////////////////////////////////////////
// AMyPlayerController
void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();
    InitSocket();
}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CleanupSocket();
	Super::EndPlay(EndPlayReason);
}

//////////////////////////////////////////////////
// Socket
void AMyPlayerController::InitSocket()
{
	g_is_host = true;
	g_skills.clear();
	g_collisions.clear();
	g_monsters.clear();

	g_s_running = true;
	g_s_skill_id = 0;
	g_s_monster_id = 0;

	// [Client] : WSAStartup
	WSADATA WSAData;
	auto ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret) { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Succeed")); }

	g_h_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == g_h_socket) { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Succeed")); }

	// [Host] : WSASocket
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == g_s_socket) { UE_LOG(LogTemp, Warning, TEXT("Host Socket Create Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Host Socket Create Succeed")); }

	u_long noblock = 1;
	ioctlsocket(g_s_socket, FIONBIO, &noblock);

	// Bind & Listen
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(HOST_PORT);

	if (!FParse::Value(FCommandLine::Get(), TEXT("ip="), HOST_ADDRESS)) {
		HOST_ADDRESS = TEXT("127.0.0.1"); 
		UE_LOG(LogTemp, Warning, TEXT("No IP Specified. Using Loopback : %s"), *HOST_ADDRESS);
		g_is_host = true;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Parsed IP from Command Line : %s"), *HOST_ADDRESS);
		g_is_host = false;
	}

	std::string IP_ADDRESS = TCHAR_TO_UTF8(*HOST_ADDRESS);
	inet_pton(AF_INET, IP_ADDRESS.c_str(), &addr.sin_addr);

	if (g_is_host) {
		ret = bind(g_s_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN));
		if (SOCKET_ERROR == ret) {
			UE_LOG(LogTemp, Warning, TEXT("Bind Failed : %d"), WSAGetLastError());
			closesocket(g_s_socket);
		} else {
			UE_LOG(LogTemp, Warning, TEXT("Bind Succeed"));
			g_s_thread = std::thread(server_thread);
		}
	}

	// Connect to host
	ret = WSAConnect(g_h_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == ret) { UE_LOG(LogTemp, Warning, TEXT("WSAConnect Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("WSAConnect Succeed")); }

	DWORD recv_bytes;
	DWORD recv_flag = 0;
	ret = WSARecv(g_h_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, c_recv_callback);
}

void AMyPlayerController::CleanupSocket()
{
	g_s_running = false;

	if (g_is_host) {
		// Join Server Thread
		g_s_thread.join();

		g_monsters.clear();

		// Delete SESSION
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id] = nullptr;
			}
		}
	}

	// Delete Player
	for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
		if (g_c_players[client_id]) {
			g_c_players[client_id]->Destroy();
			g_c_players[client_id] = nullptr;
		}
	}

	closesocket(g_h_socket);
	WSACleanup();
}

//////////////////////////////////////////////////
// Server Thread
void spawn_monster(FVector Location) {
	AsyncTask(ENamedThreads::GameThread, [Location]() {
		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;

		FVector SpawnLocation(Location.X, Location.Y, Location.Z);
		FRotator SpawnRotation = FRotator::ZeroRotator;

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		// Load Monster BP Class
		UClass* MonsterBPClass = LoadClass<AEnemyCharacter>(
			nullptr,
			TEXT("/Game/Slime/BP_Slime.BP_Slime_C")
		);

		if (!MonsterBPClass) {
			UE_LOG(LogTemp, Error, TEXT("Failed to load BP_MonsterCharacter!"));
		}

		AEnemyCharacter* NewMonster = World->SpawnActor<AEnemyCharacter>(
			MonsterBPClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

		if (!NewMonster) {
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn Monster!"));
		}

		NewMonster->set_id(g_s_monster_id++);

		// AI Controller
		AEnemyAIController* NewAI = World->SpawnActor<AEnemyAIController>(
			AEnemyAIController::StaticClass(),
			SpawnLocation,
			SpawnRotation,
			Params
		);

		if (NewAI) {
			NewAI->Possess(NewMonster);
			UE_LOG(LogTemp, Warning, TEXT("AEnemyAIController Possessed Monster"));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn AEnemyAIController"));
		}

		// Run BT
		UBehaviorTree* BTAsset = LoadObject<UBehaviorTree>(
			nullptr,
			TEXT("/Game/Slime/AI/BT_EnemyAI.BT_EnemyAI")
		);

		if (BTAsset) {
			APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);

			NewAI->RunBehaviorTree(BTAsset);

			NewAI->GetBlackboardComponent()->SetValueAsVector(TEXT("StartLocation"), NewMonster->GetActorLocation());
			NewAI->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), PlayerPawn);

			UE_LOG(LogTemp, Warning, TEXT("BehaviorTree Loaded and Running"));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Failed to Load BehaviorTree"));
		}

		// Load Animation Instance
		UClass* AnimClass = LoadClass<UAnimInstance>(
			nullptr,
			TEXT("/Game/Slime/slime/anim/BP_AnimSlime.BP_AnimSlime_C")
		);

		if (AnimClass) {
			NewMonster->GetMesh()->SetAnimInstanceClass(AnimClass);
			NewMonster->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			NewMonster->GetMesh()->SetComponentTickEnabled(true);
			NewMonster->GetMesh()->bPauseAnims = false;
			NewMonster->GetMesh()->bNoSkeletonUpdate = false;

			UE_LOG(LogTemp, Warning, TEXT("AnimInstance Set: %s"), *AnimClass->GetName());
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Failed to Load AnimBP"));
		}

		g_monsters[NewMonster->get_id()] = NewMonster;
		UE_LOG(LogTemp, Warning, TEXT("[Client] Spawned Monster %d and Stored in g_s_monsters"), NewMonster->get_id());
	});
}

void server_thread() {
	// Create Accept Thread
	std::thread a_thread(accept_thread);

	auto last_update_t = std::chrono::system_clock::now();
	auto last_packet_t = std::chrono::system_clock::now();

	spawn_monster(FVector(32'500, -40'000, 400));
	spawn_monster(FVector(32'750, -39'000, 400));
	spawn_monster(FVector(33'000, -38'000, 400));

	while (g_s_running) {
		// Game Logic
		auto curr_t = std::chrono::system_clock::now();
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_update_t).count();

		if (16 <= exec_ms) {
			/*
				Game Logic
			*/

			last_update_t = std::chrono::system_clock::now();
		}

		// WSASend
		exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_packet_t).count();

		if (16 <= exec_ms) {
			hc_monster_packet p;
			p.packet_size = sizeof(hc_monster_packet);
			p.packet_type = H2C_MONSTER_PACKET;
			
			for (const auto& [monster_id, ptr] : g_monsters) {
				AEnemyCharacter* monster = Cast< AEnemyCharacter>(ptr);

				if (monster->get_is_attacking()) {
					continue;
				}

				FVector Location = ptr->GetActorLocation();
				FVector Velocity = ptr->GetVelocity();
				FRotator Rotation = ptr->GetActorRotation();

				p.monster_id = monster_id;
				p.monster_hp = monster->get_hp();
				p.monster_x = Location.X; p.monster_y = Location.Y; p.monster_z = Location.Z;
				p.monster_vx = Velocity.X; p.monster_vy = Velocity.Y; p.monster_vz = Velocity.Z;
				p.monster_yaw = Rotation.Yaw;

				for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
					if (other_id) {
						if (g_s_clients[other_id]) {
							if (ST_INGAME == g_s_clients[other_id]->m_state) {
								g_s_clients[other_id]->do_send(&p);
							}
						}
					}
				}
			}

			last_packet_t = std::chrono::system_clock::now();
		}
	}

	a_thread.join();
}

//////////////////////////////////////////////////
// Accept Thread
void accept_thread() {
	auto ret = listen(g_s_socket, SOMAXCONN);
	if (SOCKET_ERROR == ret) { UE_LOG(LogTemp, Warning, TEXT("Listen Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("Listen Succeed")); }

	SOCKADDR_IN addr;
	INT addr_size = sizeof(SOCKADDR_IN);

	// Accept Loop
	while (g_s_running) {
		// WSAAccept
		auto c_socket = WSAAccept(g_s_socket, reinterpret_cast<sockaddr*>(&addr), &addr_size, NULL, NULL);
		if (INVALID_SOCKET == c_socket) { 
			int err = WSAGetLastError();
			if (WSAEWOULDBLOCK == err) {
				SleepEx(1, TRUE); 
				continue;
			}
			else {
				UE_LOG(LogTemp, Error, TEXT("WSAAccept Failed : %d"), err);
				break;
			}
		}
		else { UE_LOG(LogTemp, Warning, TEXT("WSAAccept Succeed")); }

		// Create SESSION
		bool slot_found = false;

		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (!g_s_clients[client_id]) {
				g_s_clients[client_id] = std::make_unique<SESSION>(client_id, c_socket, h_recv_callback, h_send_callback);
				UE_LOG(LogTemp, Warning, TEXT("[Host] Player %d Connected to Server"), g_s_clients[client_id]->m_id);

				{
					// Send Player Info to Player
					hc_player_info_packet p;
					p.packet_size = sizeof(hc_player_info_packet);
					p.packet_type = H2C_PLAYER_INFO_PACKET;
					p.id = client_id;
					p.yaw = 0.0f;
					p.x = 37'975.0f; p.y = -40'000.0f; p.z = 950.0f;
					p.vx = 0.0f; p.vy = 0.0f; p.vz = 0.0f;
					p.hp = 100;
					p.current_element = static_cast<char>(EClassType::CT_Wind);
					g_s_clients[client_id]->do_send(&p);

					// Send Player Info to Others
					p.packet_type = H2C_PLAYER_ENTER_PACKET;
					for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
						if (client_id != other_id) {
							if (g_s_clients[other_id]) {
								g_s_clients[other_id]->do_send(&p);
							}
						}
					}

					// Send Other Infos to Player
					for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
						if (client_id != other_id) {
							if (g_s_clients[other_id]) {
								APlayerCharacter* client = g_c_players[other_id];
								p.id = client->get_id();
								p.yaw = client->get_yaw();
								p.x = client->GetActorLocation().X; p.y = client->GetActorLocation().Y; p.z = client->GetActorLocation().Z;
								p.vx = client->get_velocity().X; p.vy = client->get_velocity().Y; p.vz = client->get_velocity().Z;
								p.hp = client->get_hp();
								p.current_element = client->get_current_element();
								g_s_clients[client_id]->do_send(&p);
							}
						}
					}
				}

				{
					// Send Monster Infos to Player
					unsigned short monster_count = g_monsters.size();
					unsigned char packet_size = sizeof(hc_init_monster_packet) + (sizeof(monster_info) * monster_count);

					hc_init_monster_packet* p = (hc_init_monster_packet*)malloc(packet_size);
					p->packet_size = packet_size;
					p->packet_type = H2C_INIT_MONSTER_PACKET;
					p->client_id = client_id;
					p->monster_count = monster_count;

					monster_info* monster_data = (monster_info*)(p + 1);
					unsigned short i = 0;
					for (const auto& [monster_id, ptr] : g_monsters) {
						AEnemyCharacter* monster = Cast< AEnemyCharacter>(ptr);

						if (monster->get_is_attacking()) {
							continue;
						}

						FVector Location = ptr->GetActorLocation();
						FVector Velocity = ptr->GetVelocity();
						FRotator Rotation = ptr->GetActorRotation();

						monster_data[i].monster_id = monster_id;
						monster_data[i].monster_hp = monster->get_hp();
						monster_data[i].monster_x = Location.X; monster_data[i].monster_y = Location.Y; monster_data[i].monster_z = Location.Z;
						monster_data[i].monster_vx = Velocity.X; monster_data[i].monster_vy = Velocity.Y; monster_data[i].monster_vz = Velocity.Z;
						monster_data[i].monster_yaw = Rotation.Yaw;
						++i;
					}
					g_s_clients[client_id]->do_send(p);
					free(p);
				}

				slot_found = true;
				break;
			} 
		}

		if (!slot_found) {
			closesocket(c_socket);
		}
	}

	closesocket(g_s_socket);
}





//////////////////////////////////////////////////
// Host CALLBACK
void h_process_packet(char* packet) {
	char packet_type = packet[1];
	//UE_LOG(LogTemp, Warning, TEXT("[Host] Received Packet Type : %d"), packet_type);

	switch (packet_type) {
	case C2H_PLAYER_VECTOR_PACKET: {
		player_vector_packet* p = reinterpret_cast<player_vector_packet*>(packet);
		p->packet_type = H2C_PLAYER_VECTOR_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Vector Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_STOP_PACKET: {
		player_stop_packet* p = reinterpret_cast<player_stop_packet*>(packet);
		p->packet_type = H2C_PLAYER_STOP_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Stop Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_ROTATE_PACKET: {
		player_rotate_packet* p = reinterpret_cast<player_rotate_packet*>(packet);
		p->packet_type = H2C_PLAYER_ROTATE_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Rotation Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_JUMP_PACKET: {
		player_jump_packet* p = reinterpret_cast<player_jump_packet*>(packet);
		p->packet_type = H2C_PLAYER_JUMP_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Jump Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_PLAYER_SKILL_VECTOR_PACKET: {
		ch_player_skill_vector_packet* ch_p = reinterpret_cast<ch_player_skill_vector_packet*>(packet);
		hc_player_skill_vector_packet hc_p;
		hc_p.packet_size = sizeof(hc_player_skill_vector_packet);
		hc_p.packet_type = H2C_PLAYER_SKILL_VECTOR_PACKET;
		hc_p.player_id = ch_p->player_id;
		hc_p.skill_id = g_s_skill_id++;
		hc_p.skill_type = ch_p->skill_type;
		hc_p.x = ch_p->x; hc_p.y = ch_p->y; hc_p.z = ch_p->z;
		hc_p.is_left = ch_p->is_left;
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(&hc_p);
				//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Skill Packet to Player %d"), ch_p->player_id, client_id);
			}
		}
		break;
	}

	case C2H_PLAYER_SKILL_ROTATOR_PACKET: {
		ch_player_skill_rotator_packet* ch_p = reinterpret_cast<ch_player_skill_rotator_packet*>(packet);
		hc_player_skill_rotator_packet hc_p;
		hc_p.packet_size = sizeof(hc_player_skill_rotator_packet);
		hc_p.packet_type = H2C_PLAYER_SKILL_ROTATOR_PACKET;
		hc_p.player_id = ch_p->player_id;

		switch (ch_p->skill_type) {
		case SKILL_FIRE_WALL:
			hc_p.skill_id = g_s_skill_id.fetch_add(5);
			break;

		default:
			hc_p.skill_id = g_s_skill_id++;
			break;
		}

		hc_p.skill_type = ch_p->skill_type;
		hc_p.x = ch_p->x; hc_p.y = ch_p->y; hc_p.z = ch_p->z;
		hc_p.pitch = ch_p->pitch; hc_p.yaw = ch_p->yaw; hc_p.roll = ch_p->roll;
		hc_p.is_left = ch_p->is_left;
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(&hc_p);
				//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Skill Packet to Player %d"), p->player_id, client_id);
			}
		}
		break;
	}

	case C2H_PLAYER_CHANGE_ELEMENT_PACKET: {
		player_change_element_packet* p = reinterpret_cast<player_change_element_packet*>(packet);
		p->packet_type = H2C_PLAYER_CHANGE_ELEMENT_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
					//UE_LOG(LogTemp, Warning, TEXT("[Host] Send Player %d's Skill Packet to Player %d"), p->id, other_id);
				}
			}
		}
		break;
	}

	case C2H_COLLISION_PACKET: {
		collision_packet* p = reinterpret_cast<collision_packet*>(packet);
		p->packet_type = H2C_COLLISION_PACKET;
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(p);
			}
		}
		//UE_LOG(LogTemp, Warning, TEXT("[Host] %d and %d Collision"), p->attacker_id, p->victim_id);
		break;
	}

	case C2H_SKILL_CREATE_PACKET: {
		ch_skill_create_packet* ch_p = reinterpret_cast<ch_skill_create_packet*>(packet);
		hc_skill_create_packet hc_p;
		hc_p.packet_size = sizeof(hc_skill_create_packet);
		hc_p.packet_type = H2C_SKILL_CREATE_PACKET;
		hc_p.skill_type = ch_p->skill_type;
		hc_p.old_skill_id = ch_p->old_skill_id;
		hc_p.new_skill_id = g_s_skill_id++;
		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(&hc_p);
			}
		}
		//UE_LOG(LogTemp, Warning, TEXT("[Host] Skill %d Create"), p->new_skill_id);
		break;
	}

	case C2H_INIT_COMPLETE_PACKET: {
		ch_init_complete_packet* p = reinterpret_cast<ch_init_complete_packet*>(packet);
		g_s_clients[p->client_id]->m_state = ST_INGAME;
		//UE_LOG(LogTemp, Warning, TEXT("[Host] Client %d Init Complete"), p->client_id);
		break;
	}

	case C2H_MONSTER_ATTACK_PACKET: {
		monster_attack_packet* p = reinterpret_cast<monster_attack_packet*>(packet);
		p->packet_type = H2C_MONSTER_ATTACK_PACKET;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
				}
			}
		}
		//UE_LOG(LogTemp, Warning, TEXT("[Host] Skill %d Create"), p->new_skill_id);
		break;
	}
	}
}

extern void CALLBACK h_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	//UE_LOG(LogTemp, Warning, TEXT("[Host] h_recv_callback"));

	SESSION* o = reinterpret_cast<SESSION*>(p_over);
	if (!o) { return; }
	//UE_LOG(LogTemp, Warning, TEXT("[Host] Received Packet from Player %d"), o->m_id);

	if ((0 != err) || (0 == num_bytes)) {
		UE_LOG(LogTemp, Warning, TEXT("[Host] Player %d Disconnected"), o->m_id);

		hc_player_leave_packet p;
		p.packet_size = sizeof(hc_player_leave_packet);
		p.packet_type = H2C_PLAYER_LEAVE_PACKET;
		p.id = g_s_clients[o->m_id]->m_id;

		g_s_clients[p.id] = nullptr;
		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (g_s_clients[other_id]) {
				g_s_clients[other_id]->do_send(&p);
			}
		}
		return;
	}

	// Process Packet
	char* p = o->m_recv_over.m_buffer;
	unsigned char packet_size = p[0];
	int remained = o->m_remained + num_bytes;

	while (packet_size <= remained) {
		h_process_packet(p);
		p += packet_size;
		remained -= packet_size;
		if (!remained) {
			break;
		}
		packet_size = p[0];
	}

	o->m_remained = remained;
	if (remained) {
		memcpy(o->m_recv_over.m_buffer, p, remained);
	}

	o->do_recv();
}

extern void CALLBACK h_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	//UE_LOG(LogTemp, Warning, TEXT("[Host] h_send_callback"));
	
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}





//////////////////////////////////////////////////
// Client CALLBACK
void c_process_packet(char* packet) {
	char packet_type = packet[1];
	//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Packet Type : %d"), packet_type);

	switch (packet_type) {
	case H2C_PLAYER_INFO_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);

		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;

		APlayerCharacter* MyPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
		if (MyPlayer) {
			MyPlayer->set_id(p->id);
			MyPlayer->set_yaw(p->yaw);
			MyPlayer->SetActorLocation(FVector(p->x, p->y, p->z));
			MyPlayer->set_velocity(p->vx, p->vy, p->vz);
			MyPlayer->set_is_player(true);
			MyPlayer->set_hp(p->hp);
			MyPlayer->set_current_element(p->current_element);

			g_c_players[p->id] = MyPlayer;

			//UE_LOG(LogTemp, Warning, TEXT("[Client] Registered Local Player (ID: %d) in g_c_players"), p->id);
		}
		break;
	}

	case H2C_PLAYER_ENTER_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);
		UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d has Entered the Game"), p->id);
	
		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;
	
		FVector SpawnLocation(37'975.0f, -40'000.0f, 950.0f); // 초기 위치 설정
		FRotator SpawnRotation = FRotator::ZeroRotator;
	
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
		UClass* PlayerBPClass = LoadClass<APlayerCharacter>(
			nullptr,
			TEXT("/Game/player_anim/MyPlayerCharacter.MyPlayerCharacter_C")
		);

		if (!PlayerBPClass) {
			UE_LOG(LogTemp, Error, TEXT("Failed to load BP_PlayerCharacter!"));
			return;
		}

		APlayerCharacter* NewPlayer = World->SpawnActor<APlayerCharacter>(
			PlayerBPClass,
			SpawnLocation,
			SpawnRotation,
			Params
		);

		NewPlayer->set_id(p->id);
		NewPlayer->set_yaw(p->yaw);
		NewPlayer->SetActorLocation(FVector(p->x, p->y, p->z));
		NewPlayer->set_velocity(p->vx, p->vy, p->vz);
		NewPlayer->set_is_player(true);
		NewPlayer->set_hp(p->hp);
		NewPlayer->set_current_element(p->current_element);
		NewPlayer->set_is_player(false);

		AAIController* NewAI = World->SpawnActor<AAIController>(
			AAIController::StaticClass(),
			SpawnLocation,
			SpawnRotation,
			Params
		);

		if (NewAI) {
			NewAI->Possess(NewPlayer);
			UE_LOG(LogTemp, Warning, TEXT("AIController Possessed Avatar"));
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn AIController"));
		}

		UClass* AnimClass = LoadClass<UAnimInstance>(
			nullptr,
			TEXT("/Game/player_anim/MyPlayerAnim.MyPlayerAnim_C")
		);
	
		if (AnimClass)
		{
			NewPlayer->GetMesh()->SetAnimInstanceClass(AnimClass);
			NewPlayer->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			NewPlayer->GetMesh()->SetComponentTickEnabled(true);
			NewPlayer->GetMesh()->bPauseAnims = false;
			NewPlayer->GetMesh()->bNoSkeletonUpdate = false;
	
			UE_LOG(LogTemp, Warning, TEXT("AnimInstance Set: %s"), *AnimClass->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to Load AnimBP"));
		}


		g_c_players[p->id] = NewPlayer;
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Spawned Player %d and Stored in g_c_players"), p->id);
		break;
	}

	case H2C_PLAYER_LEAVE_PACKET: {
		hc_player_leave_packet* p = reinterpret_cast<hc_player_leave_packet*>(packet);
		UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d has Leaved the Game"), p->id);

		g_c_players[p->id]->Destroy();
		g_c_players[p->id] = nullptr;
		break;
	}

	case H2C_PLAYER_VECTOR_PACKET: {
		player_vector_packet* p = reinterpret_cast<player_vector_packet*>(packet);
		FVector Position(p->x, p->y, p->z);
		FVector Velocity(p->vx, p->vy, p->vz);
		g_c_players[p->id]->SetActorLocation(Position, false);
		g_c_players[p->id]->set_velocity(Velocity.X, Velocity.Y, Velocity.Z);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Player %d Moved, vx : %.2f, vy : %.2f"), p->id, p->vx, p->vy);
		break;
	}

	case H2C_PLAYER_STOP_PACKET: {
		player_stop_packet* p = reinterpret_cast<player_stop_packet*>(packet);
		FVector Position(p->x, p->y, p->z);
		g_c_players[p->id]->SetActorLocation(Position, false);
		g_c_players[p->id]->set_velocity(0.0f, 0.0f, 0.0f);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Stop Packet"), p->id);
		break;
	}

	case H2C_PLAYER_ROTATE_PACKET: {
		player_rotate_packet* p = reinterpret_cast<player_rotate_packet*>(packet);
		g_c_players[p->id]->rotate(p->yaw);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Rotation Packet"), p->id);
		break;
	}

	case H2C_PLAYER_JUMP_PACKET: {
		player_jump_packet* p = reinterpret_cast<player_jump_packet*>(packet);
		g_c_players[p->id]->LaunchCharacter(FVector(0, 0, 800), false, true);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Jump Packet"), p->id);
		break;
	}

	case H2C_PLAYER_SKILL_VECTOR_PACKET: {
		hc_player_skill_vector_packet* p = reinterpret_cast<hc_player_skill_vector_packet*>(packet);
		g_c_players[p->player_id]->use_skill(p->skill_id, p->skill_type, FVector(p->x, p->y, p->z), p->is_left);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Skill %d Packet"), p->player_id, p->skill_id);
		break;
	}

	case H2C_PLAYER_SKILL_ROTATOR_PACKET: {
		hc_player_skill_rotator_packet* p = reinterpret_cast<hc_player_skill_rotator_packet*>(packet);
		g_c_players[p->player_id]->use_skill(p->skill_id, p->skill_type, FVector(p->x, p->y, p->z), FRotator(p->pitch, p->yaw, p->roll), p->is_left);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Skill %d Packet"), p->player_id, p->skill_id);
		break;
	}

	case H2C_PLAYER_CHANGE_ELEMENT_PACKET: {
		player_change_element_packet* p = reinterpret_cast<player_change_element_packet*>(packet);
		g_c_players[p->id]->change_element(p->element_type, p->is_left);
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Player %d's Skill Packet"), p->player_id);
		break;
	}

	case H2C_COLLISION_PACKET: {
		collision_packet* p = reinterpret_cast<collision_packet*>(packet);
		switch (p->collision_type) {
		case SKILL_SKILL_COLLISION:
			if (g_skills.count(p->attacker_id) && g_skills.count(p->victim_id)) {
				g_skills[p->attacker_id]->Overlap(g_skills[p->victim_id]);
				g_skills[p->victim_id]->Overlap(g_skills[p->attacker_id]);
				//UE_LOG(LogTemp, Error, TEXT("[Client] Skill %d and %d Collision"), p->attacker_id, p->victim_id);
			}
			else {
				g_collisions[p->attacker_id].push(p->victim_id);
				g_collisions[p->victim_id].push(p->attacker_id);
				//UE_LOG(LogTemp, Error, TEXT("[Client] Skill %d and %d Delayed"), p->attacker_id, p->victim_id);
			}
			break;

		case SKILL_MONSTER_COLLISION:
			if (g_skills.count(p->attacker_id) && g_monsters.count(p->victim_id)) {
				g_skills[p->attacker_id]->Overlap(g_monsters[p->victim_id]);
				Cast<AEnemyCharacter>(g_monsters[p->victim_id])->Overlap(g_skills[p->attacker_id]);
			}
			//UE_LOG(LogTemp, Error, TEXT("[Client] Skill %d and Monster %d Collision"), p->attacker_id, p->victim_id);
			break;

		case SKILL_PLAYER_COLLISION:
			if (g_skills.count(p->attacker_id) && g_c_players[p->victim_id]) {
				g_skills[p->attacker_id]->Overlap(g_monsters[p->victim_id]);
			}
			//UE_LOG(LogTemp, Error, TEXT("[Client] Skill %d and Player %d Collision"), p->attacker_id, p->victim_id);
			break;
		}
	}

	case H2C_SKILL_CREATE_PACKET: {
		hc_skill_create_packet* p = reinterpret_cast<hc_skill_create_packet*>(packet);
		switch (p->skill_type) {
		case SKILL_WIND_FIRE_BOMB:
			if (g_skills.count(p->old_skill_id)) {
				if (g_skills[p->old_skill_id]->IsA(AMyWindCutter::StaticClass())) {
					Cast<AMyWindCutter>(g_skills[p->old_skill_id])->MixBombAttack(EClassType::CT_Fire, p->new_skill_id);
				}
			}
			break;

		case SKILL_WIND_WIND_TORNADO:
			if (g_skills.count(p->old_skill_id)) {
				if (g_skills[p->old_skill_id]->IsA(AMyWindSkill::StaticClass())) {
					Cast<AMyWindSkill>(g_skills[p->old_skill_id])->SpawnMixTonado(p->new_skill_id);
				}
			}
			break;
		}
		//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Collision Packet"));
		break;
	}

	case H2C_INIT_MONSTER_PACKET: {
		hc_init_monster_packet* p = reinterpret_cast<hc_init_monster_packet*>(packet);
		char client_id = p->client_id;
		unsigned short monster_count = 0;
		unsigned short expected_count = p->monster_count;

		for (unsigned short i = 0; i < expected_count; ++i) {
			monster_info info = p->monsters[i];

			AsyncTask(ENamedThreads::GameThread, [info, monster_count, expected_count, client_id]() {
				UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
				if (!World) return;

				FVector SpawnLocation(info.monster_x, info.monster_y, info.monster_z);
				FRotator SpawnRotation = FRotator::ZeroRotator;

				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// Load Monster BP Class
				UClass* MonsterBPClass = LoadClass<AEnemyCharacter>(
					nullptr,
					TEXT("/Game/Slime/BP_Slime.BP_Slime_C")
				);

				if (!MonsterBPClass) {
					UE_LOG(LogTemp, Error, TEXT("Failed to load BP_MonsterCharacter!"));
				}

				AEnemyCharacter* NewMonster = World->SpawnActor<AEnemyCharacter>(
					MonsterBPClass,
					SpawnLocation,
					SpawnRotation,
					Params
				);

				if (!NewMonster) {
					UE_LOG(LogTemp, Error, TEXT("Failed to spawn Monster!"));
				}

				FRotator NewRotation(0.f, info.monster_yaw, 0.f);

				NewMonster->set_id(info.monster_id);
				NewMonster->set_hp(info.monster_hp);
				NewMonster->SetActorLocation(FVector(info.monster_x, info.monster_y, info.monster_z));
				NewMonster->GetCharacterMovement()->Velocity = FVector(info.monster_vx, info.monster_vy, info.monster_vz);
				NewMonster->SetActorRotation(NewRotation);

				// AI Controller
				AEnemyAIController* NewAI = World->SpawnActor<AEnemyAIController>(
					AEnemyAIController::StaticClass(),
					SpawnLocation,
					SpawnRotation,
					Params
				);

				if (NewAI) {
					NewAI->Possess(NewMonster);
					UE_LOG(LogTemp, Warning, TEXT("AEnemyAIController Possessed Monster"));
				}
				else {
					UE_LOG(LogTemp, Error, TEXT("Failed to spawn AEnemyAIController"));
				}

				// Load Animation Instance
				UClass* AnimClass = LoadClass<UAnimInstance>(
					nullptr,
					TEXT("/Game/Slime/slime/anim/BP_AnimSlime.BP_AnimSlime_C")
				);

				if (AnimClass) {
					NewMonster->GetMesh()->SetAnimInstanceClass(AnimClass);
					NewMonster->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
					NewMonster->GetMesh()->SetComponentTickEnabled(true);
					NewMonster->GetMesh()->bPauseAnims = false;
					NewMonster->GetMesh()->bNoSkeletonUpdate = false;

					UE_LOG(LogTemp, Warning, TEXT("AnimInstance Set: %s"), *AnimClass->GetName());
				}
				else {
					UE_LOG(LogTemp, Error, TEXT("Failed to Load AnimBP"));
				}

				g_monsters[info.monster_id] = NewMonster;

				if (monster_count == (expected_count - 1)) {
					ch_init_complete_packet p;
					p.packet_size = sizeof(ch_init_complete_packet);
					p.packet_type = C2H_INIT_COMPLETE_PACKET;
					p.client_id = client_id;

					g_c_players[client_id]->do_send(&p);
				}
				UE_LOG(LogTemp, Warning, TEXT("[Client] Spawned Monster %d and Stored in g_s_monsters"), info.monster_id);
			});

			++monster_count;
		}
		break;
	}

	case H2C_MONSTER_PACKET: {
		hc_monster_packet p = *reinterpret_cast<hc_monster_packet*>(packet);

		if (!g_monsters.count(p.monster_id)) {
			AsyncTask(ENamedThreads::GameThread, [p]() {
				UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
				if (!World) return;

				FVector SpawnLocation(p.monster_x, p.monster_y, p.monster_z);
				FRotator SpawnRotation = FRotator::ZeroRotator;

				FActorSpawnParameters Params;
				Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				// Load Monster BP Class
				UClass* MonsterBPClass = LoadClass<AEnemyCharacter>(
					nullptr,
					TEXT("/Game/Slime/BP_Slime.BP_Slime_C")
				);

				if (!MonsterBPClass) {
					UE_LOG(LogTemp, Error, TEXT("Failed to load BP_MonsterCharacter!"));
				}

				AEnemyCharacter* NewMonster = World->SpawnActor<AEnemyCharacter>(
					MonsterBPClass,
					SpawnLocation,
					SpawnRotation,
					Params
				);

				if (!NewMonster) {
					UE_LOG(LogTemp, Error, TEXT("Failed to spawn Monster!"));
				}

				NewMonster->set_id(p.monster_id);

				// AI Controller
				AEnemyAIController* NewAI = World->SpawnActor<AEnemyAIController>(
					AEnemyAIController::StaticClass(),
					SpawnLocation,
					SpawnRotation,
					Params
				);

				if (NewAI) {
					NewAI->Possess(NewMonster);
					UE_LOG(LogTemp, Warning, TEXT("AEnemyAIController Possessed Monster"));
				}
				else {
					UE_LOG(LogTemp, Error, TEXT("Failed to spawn AEnemyAIController"));
				}

				// Load Animation Instance
				UClass* AnimClass = LoadClass<UAnimInstance>(
					nullptr,
					TEXT("/Game/Slime/slime/anim/BP_AnimSlime.BP_AnimSlime_C")
				);

				if (AnimClass) {
					NewMonster->GetMesh()->SetAnimInstanceClass(AnimClass);
					NewMonster->GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
					NewMonster->GetMesh()->SetComponentTickEnabled(true);
					NewMonster->GetMesh()->bPauseAnims = false;
					NewMonster->GetMesh()->bNoSkeletonUpdate = false;

					UE_LOG(LogTemp, Warning, TEXT("AnimInstance Set: %s"), *AnimClass->GetName());
				}
				else {
					UE_LOG(LogTemp, Error, TEXT("Failed to Load AnimBP"));
				}

				g_monsters[p.monster_id] = NewMonster;
				UE_LOG(LogTemp, Warning, TEXT("[Client] Spawned Monster %d and Stored in g_s_monsters"), p.monster_id);
				});
		} else {
			AEnemyCharacter* Monster = Cast<AEnemyCharacter>(g_monsters[p.monster_id]);
			FRotator NewRotation(0.f, p.monster_yaw, 0.f);

			Monster->set_hp(p.monster_hp);
			Monster->SetActorLocation(FVector(p.monster_x, p.monster_y, p.monster_z));
			Monster->GetCharacterMovement()->Velocity = FVector(p.monster_vx, p.monster_vy, p.monster_vz);
			Monster->SetActorRotation(NewRotation);
		}
		UE_LOG(LogTemp, Warning, TEXT("[Client] Received Monster %d Packet, X : %.2f, Y : %.2f"), p.monster_id, p.monster_x, p.monster_y);
		break;
	}

	case H2C_MONSTER_ATTACK_PACKET: {
		hc_monster_packet* p = reinterpret_cast<hc_monster_packet*>(packet);

		if (g_monsters.count(p->monster_id)) {
			Cast<AEnemyCharacter>(g_monsters[p->monster_id])->MeleeAttack();
		}

		UE_LOG(LogTemp, Warning, TEXT("[Client] Received Monster %d Attack Packet"), p->monster_id);
		break;
	}
	}
}

void CALLBACK c_recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	if ((0 != err) || (0 == num_bytes)) {
		UE_LOG(LogTemp, Warning, TEXT("[Client] Server Disconnected"));
		return;
	}

	// Process Packet
	//UE_LOG(LogTemp, Warning, TEXT("[Client] Received Packet from Host"));
	char* p = g_recv_over.m_buffer;
	unsigned char packet_size = p[0];
	int remained = g_remained + num_bytes;

	while (packet_size <= remained) {
		c_process_packet(p);
		p += packet_size;
		remained -= packet_size;
		if (!remained) {
			break;
		}
		packet_size = p[0];
	}

	g_remained = remained;
	if (remained) {
		memcpy(g_recv_over.m_buffer, p, remained);
	}

	DWORD recv_bytes;
	DWORD recv_flag = 0;
	g_recv_over.m_wsabuf[0].buf = g_recv_over.m_buffer + g_remained;
	g_recv_over.m_wsabuf[0].len = sizeof(g_recv_over.m_buffer) - g_remained;
	WSARecv(g_h_socket, g_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_recv_over.m_over, c_recv_callback);
}

void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}
