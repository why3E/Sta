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

constexpr short HOST_PORT = 3000;
FString HOST_ADDRESS = TEXT("127.0.0.1");

//////////////////////////////////////////////////
// Server
SOCKET g_s_socket;
std::thread g_s_thread;
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
EXP_OVER g_c_recv_over;
int g_c_remained;

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
	g_is_running = true;

	g_c_skills.clear();
	g_c_monsters.clear();
	g_c_collisions.clear();

	g_s_skill_id = 0;
	g_s_monster_id = 0;

	// [Client] : WSAStartup
	WSADATA WSAData;
	auto ret = WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (0 != ret) { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("WSAStartup Succeed")); }

	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (INVALID_SOCKET == g_c_socket) { UE_LOG(LogTemp, Warning, TEXT("Client Socket Create Failed : %d"), WSAGetLastError()); }
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
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (!FParse::Value(FCommandLine::Get(), TEXT("ip="), HOST_ADDRESS)) {
		HOST_ADDRESS = TEXT("127.0.0.1"); 
		UE_LOG(LogTemp, Warning, TEXT("No IP Specified. Using Loopback : %s"), *HOST_ADDRESS);
		g_is_host = true;
	} else {
		UE_LOG(LogTemp, Warning, TEXT("Parsed IP from Command Line : %s"), *HOST_ADDRESS);
		g_is_host = false;
	}

	if (g_is_host) {
		ret = bind(g_s_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN));
		if (SOCKET_ERROR == ret) {
			UE_LOG(LogTemp, Warning, TEXT("Bind Failed : %d"), WSAGetLastError());
			g_is_host = false;
			closesocket(g_s_socket);
		} else {
			UE_LOG(LogTemp, Warning, TEXT("Bind Succeed"));
			g_s_thread = std::thread(server_thread);
		}
	}

	std::string IP_ADDRESS = TCHAR_TO_UTF8(*HOST_ADDRESS);
	inet_pton(AF_INET, IP_ADDRESS.c_str(), &addr.sin_addr);

	// Connect to host
	ret = WSAConnect(g_c_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(SOCKADDR_IN), NULL, NULL, NULL, NULL);
	if (SOCKET_ERROR == ret) { UE_LOG(LogTemp, Warning, TEXT("WSAConnect Failed : %d"), WSAGetLastError()); }
	else { UE_LOG(LogTemp, Warning, TEXT("WSAConnect Succeed")); }

	DWORD recv_bytes;
	DWORD recv_flag = 0;
	ret = WSARecv(g_c_socket, g_c_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_c_recv_over.m_over, c_recv_callback);
}

void AMyPlayerController::CleanupSocket()
{
	g_is_running = false;

	if (g_is_host) {
		// Join Server Thread
		g_s_thread.join();

		g_c_skills.clear();
		g_c_monsters.clear();
		g_c_collisions.clear();

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

	closesocket(g_c_socket);
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

		g_c_monsters[NewMonster->get_id()] = NewMonster;
		UE_LOG(LogTemp, Warning, TEXT("[Client] Spawned Monster %d and Stored in g_s_monsters"), NewMonster->get_id());
	});
}

void spawn_monster_from_json() {
	FString FilePath = FPaths::ProjectContentDir() + TEXT("Data/MonsterSpawnLocations.json");
	FString JsonString;

	UE_LOG(LogTemp, Warning, TEXT("Loading JSON from: %s"), *FilePath);

	if (!FFileHelper::LoadFileToString(JsonString, *FilePath)) 	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load JSON file!"));
		return;
	}

	TSharedPtr<FJsonValue> JsonParsed;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, JsonParsed) || !JsonParsed.IsValid()) {
		UE_LOG(LogTemp, Error, TEXT("Failed to parse JSON!"));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* Coordinates;
	if (!JsonParsed->TryGetArray(Coordinates)) {
		UE_LOG(LogTemp, Error, TEXT("JSON is not an array!"));
		return;
	}

	for (const TSharedPtr<FJsonValue>& Value : *Coordinates) {
		const TSharedPtr<FJsonObject>* Obj;

		if (Value->TryGetObject(Obj)) {
			float x = (*Obj)->GetNumberField("x");
			float y = (*Obj)->GetNumberField("y");
			float z = (*Obj)->GetNumberField("z");

			spawn_monster(FVector(x, y, z));
		}
	}
}

void server_thread() {
	// Create Accept Thread
	std::thread a_thread(accept_thread);

	auto last_update_t = std::chrono::system_clock::now();
	auto last_packet_t = std::chrono::system_clock::now();

	spawn_monster_from_json();

	while (g_is_running) {
		// Game Logic
		auto curr_t = std::chrono::system_clock::now();
		auto exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_update_t).count();

		if (16 <= exec_ms) {


			last_update_t = std::chrono::system_clock::now();
		}

		// WSASend
		exec_ms = std::chrono::duration_cast<std::chrono::milliseconds>(curr_t - last_packet_t).count();

		if (16 <= exec_ms) {
			{
				std::lock_guard<std::mutex> lock(g_s_monster_events_l);

				while (!g_s_monster_events.empty()) {
					MonsterEvent monster_event = g_s_monster_events.front();
					g_s_monster_events.pop();

					switch (monster_event.event_type) {
					case EventType::Target: {
						hc_monster_move_packet p;
						p.packet_size = sizeof(hc_monster_move_packet);
						p.packet_type = H2C_MONSTER_MOVE_PACKET;
						p.id = monster_event.data.target.id;
						p.target_x = monster_event.data.target.target_location.X; p.target_y = monster_event.data.target.target_location.Y; p.target_z = monster_event.data.target.target_location.Z;

						if (g_c_monsters[p.id]) {
							if (nullptr != g_c_monsters[p.id]) {
								Cast<AEnemyCharacter>(g_c_monsters[p.id])->set_target_location(monster_event.data.target.target_location);
							}
						}

						for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
							if (client_id) {
								if (g_s_clients[client_id]) {
									g_s_clients[client_id]->do_send(&p);
								}
							}
						}
						break;
					}

					case EventType::Attack: {
						hc_monster_attack_packet p;
						p.packet_size = sizeof(hc_monster_attack_packet);
						p.packet_type = H2C_MONSTER_ATTACK_PACKET;
						p.id = monster_event.data.attack.id;

						for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
							if (client_id) {
								if (g_s_clients[client_id]) {
									g_s_clients[client_id]->do_send(&p);
								}
							}
						}
						break;
					}

					case EventType::Respawn: {
						hc_monster_respawn_packet p;
						p.packet_size = sizeof(hc_monster_respawn_packet);
						p.packet_type = H2C_MONSTER_RESPAWN_PACKET;
						p.id = monster_event.data.respawn.id;
						p.respawn_x = monster_event.data.respawn.respawn_location.X; p.respawn_y = monster_event.data.respawn.respawn_location.Y; p.respawn_z = monster_event.data.respawn.respawn_location.Z;

						for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
							if (client_id) {
								if (g_s_clients[client_id]) {
									g_s_clients[client_id]->do_send(&p);
								}
							}
						}
						break;
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
	while (g_is_running) {
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
					p.element[0] = static_cast<char>(EClassType::CT_Wind);
					p.element[1] = static_cast<char>(EClassType::CT_Fire);
					g_s_clients[client_id]->do_send(&p);

					if (client_id == 0) {
						slot_found = true;
						break;
					}

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
								p.element[0] = client->get_current_element(true);
								p.element[1] = client->get_current_element(false);
								g_s_clients[client_id]->do_send(&p);
							}
						}
					}
				}

				{
					unsigned short total = g_c_monsters.size();
					unsigned short offset = 0;

					auto iter = g_c_monsters.begin();

					while (offset < total) {
						// Send Monster Infos to Player
						unsigned short monster_count = FMath::Min((unsigned short)(total - offset), MAX_MONSTERS_PER_PACKET);
						unsigned char packet_size = sizeof(hc_init_monster_packet) + (sizeof(monster_init_info) * monster_count);

						hc_init_monster_packet* p = (hc_init_monster_packet*)malloc(packet_size);
						p->packet_size = packet_size;
						p->packet_type = H2C_INIT_MONSTER_PACKET;
						p->count = monster_count;

						monster_init_info* monster_data = (monster_init_info*)(p + 1);
						unsigned short i = 0;

						while ((i < monster_count) && (iter != g_c_monsters.end())) {
							AEnemyCharacter* monster = Cast<AEnemyCharacter>(iter->second);

							FVector Location = monster->GetActorLocation();
							FVector TargetLocation = monster->get_target_location();

							monster_data[i].id = iter->first;
							monster_data[i].hp = monster->get_hp();
							monster_data[i].x = Location.X; monster_data[i].y = Location.Y; monster_data[i].z = Location.Z;
							monster_data[i].target_x = TargetLocation.X; monster_data[i].target_y = TargetLocation.Y; monster_data[i].target_z = TargetLocation.Z;

							++i;
							++offset;
							++iter;
						}

						g_s_clients[client_id]->do_send(p);
						UE_LOG(LogTemp, Warning, TEXT("[Host] Send %d Monsters to Clinet"), monster_count);
						free(p);
					}
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
	UE_LOG(LogTemp, Warning, TEXT("[Host] Received Packet Type : %d"), packet_type);

	switch (packet_type) {
	case C2H_PLAYER_MOVE_PACKET: {
		player_move_packet* p = reinterpret_cast<player_move_packet*>(packet);
		p->packet_type = H2C_PLAYER_MOVE_PACKET;

		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (p->id != other_id) {
				if (g_s_clients[other_id]) {
					g_s_clients[other_id]->do_send(p);
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
				}
			}
		}
		break;
	}

	case C2H_SKILL_VECTOR_PACKET: {
		skill_vector_packet* p = reinterpret_cast<skill_vector_packet*>(packet);
		p->packet_type = H2C_SKILL_VECTOR_PACKET;
		p->skill_id = g_s_skill_id++;

		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(p);
			}
		}
		break;
	}

	case C2H_SKILL_ROTATOR_PACKET: {
		skill_rotator_packet* p = reinterpret_cast<skill_rotator_packet*>(packet);
		p->packet_type = H2C_SKILL_ROTATOR_PACKET;

		switch (p->skill_type) {
		case SKILL_FIRE_WALL:
			p->skill_id = g_s_skill_id.fetch_add(5);
			break;

		default:
			p->skill_id = g_s_skill_id++;
			break;
		}

		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(p);
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
				}
			}
		}
		break;
	}

	case C2H_PLAYER_READY_SKILL_PACKET: {
		player_ready_skill_packet* p = reinterpret_cast<player_ready_skill_packet*>(packet);
		p->packet_type = H2C_PLAYER_READY_SKILL_PACKET;

		for (char other_id = 0; other_id < MAX_CLIENTS; ++other_id) {
			if (g_s_clients[other_id]) {
				g_s_clients[other_id]->do_send(p);
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
		break;
	}

	case C2H_SKILL_CREATE_PACKET: {
		skill_create_packet* p = reinterpret_cast<skill_create_packet*>(packet);
		p->packet_type = H2C_SKILL_CREATE_PACKET;
		p->new_skill_id = g_s_skill_id++;

		for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
			if (g_s_clients[client_id]) {
				g_s_clients[client_id]->do_send(p);
			}
		}
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
		p.player_id = g_s_clients[o->m_id]->m_id;

		g_s_clients[p.player_id] = nullptr;
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
	UE_LOG(LogTemp, Warning, TEXT("[Client] Received Packet Type : %d"), packet_type);

	switch (packet_type) {
	case H2C_PLAYER_INFO_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);

		UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
		if (!World) return;

		APlayerCharacter* MyPlayer = Cast<APlayerCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
		if (MyPlayer) {
			MyPlayer->set_id(p->id);
			MyPlayer->rotate(p->yaw);
			MyPlayer->SetActorLocation(FVector(p->x, p->y, p->z));
			MyPlayer->set_velocity(p->vx, p->vy, p->vz);
			MyPlayer->set_is_player(true);
			MyPlayer->set_hp(p->hp);
			MyPlayer->set_current_element(p->element[0], true);
			MyPlayer->set_current_element(p->element[1], false);

			g_c_players[p->id] = MyPlayer;
		}
		break;
	}

	case H2C_PLAYER_ENTER_PACKET: {
		hc_player_info_packet* p = reinterpret_cast<hc_player_info_packet*>(packet);
	
		if (nullptr != g_c_players[p->id]) { break; }

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
		NewPlayer->rotate(p->yaw);
		NewPlayer->SetActorLocation(FVector(p->x, p->y, p->z));
		NewPlayer->set_velocity(p->vx, p->vy, p->vz);
		NewPlayer->set_is_player(true);
		NewPlayer->set_hp(p->hp);
		NewPlayer->set_current_element(p->element[0], true);
		NewPlayer->set_current_element(p->element[1], false);
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

		if (nullptr == g_c_players[p->player_id]) { break; }

		g_c_players[p->player_id]->Destroy();
		g_c_players[p->player_id] = nullptr;
		break;
	}

	case H2C_PLAYER_MOVE_PACKET: {
		player_move_packet* p = reinterpret_cast<player_move_packet*>(packet);

		if (nullptr == g_c_players[p->id]) { break; }

		FVector Position(p->x, p->y, p->z);
		FVector Velocity(p->vx, p->vy, p->vz);

		g_c_players[p->id]->SetActorLocation(Position, false);
		g_c_players[p->id]->set_velocity(Velocity.X, Velocity.Y, Velocity.Z);
		break;
	}

	case H2C_PLAYER_STOP_PACKET: {
		player_stop_packet* p = reinterpret_cast<player_stop_packet*>(packet);

		if (nullptr == g_c_players[p->id]) { break; }

		FVector Position(p->x, p->y, p->z);
		
		g_c_players[p->id]->SetActorLocation(Position, false);
		g_c_players[p->id]->set_velocity(0.0f, 0.0f, 0.0f);
		break;
	}

	case H2C_PLAYER_ROTATE_PACKET: {
		player_rotate_packet* p = reinterpret_cast<player_rotate_packet*>(packet);

		if (nullptr == g_c_players[p->id]) { break; }

		g_c_players[p->id]->rotate(p->yaw);
		break;
	}

	case H2C_PLAYER_JUMP_PACKET: {
		player_jump_packet* p = reinterpret_cast<player_jump_packet*>(packet);

		if (nullptr == g_c_players[p->id]) { break; }

		g_c_players[p->id]->LaunchCharacter(FVector(0, 0, 800), false, true);
		break;
	}

	case H2C_PLAYER_READY_SKILL_PACKET: {
		player_ready_skill_packet* p = reinterpret_cast<player_ready_skill_packet*>(packet);

		if (nullptr == g_c_players[p->id]) { break; }

		g_c_players[p->id]->ready_skill(p->is_left);
		break;
	}

	case H2C_SKILL_VECTOR_PACKET: {
		skill_vector_packet* p = reinterpret_cast<skill_vector_packet*>(packet);

		if (nullptr == g_c_players[p->player_id]) { break; }

		g_c_players[p->player_id]->use_skill(p->skill_id, p->skill_type, FVector(p->skill_vx, p->skill_vy, p->skill_vz), p->is_left);
		break;
	}

	case H2C_SKILL_ROTATOR_PACKET: {
		skill_rotator_packet* p = reinterpret_cast<skill_rotator_packet*>(packet);

		if (nullptr == g_c_players[p->player_id]) { break; }

		g_c_players[p->player_id]->use_skill(p->skill_id, p->skill_type, FVector(p->skill_x, p->skill_y, p->skill_z), FRotator(p->skill_pitch, p->skill_yaw, p->skill_roll), p->is_left);
		break;
	}

	case H2C_PLAYER_CHANGE_ELEMENT_PACKET: {
		player_change_element_packet* p = reinterpret_cast<player_change_element_packet*>(packet);

		if (nullptr == g_c_players[p->id]) { break; }

		g_c_players[p->id]->change_element(p->element, p->is_left);
		break;
	}

	case H2C_COLLISION_PACKET: {
		collision_packet* p = reinterpret_cast<collision_packet*>(packet);
		switch (p->collision_type) {
		case SKILL_SKILL_COLLISION:
			if (g_c_skills.count(p->attacker_id) && g_c_skills.count(p->victim_id)) {
				if ((nullptr != g_c_skills[p->attacker_id]) && (nullptr != g_c_skills[p->victim_id])) {
					g_c_skills[p->attacker_id]->Overlap(g_c_skills[p->victim_id]);
					g_c_skills[p->victim_id]->Overlap(g_c_skills[p->attacker_id]);
				}
			} else {
				g_c_collisions[p->attacker_id].push(p->victim_id);
				g_c_collisions[p->victim_id].push(p->attacker_id);
			}
			break;

		case SKILL_MONSTER_COLLISION:
			if (g_c_skills.count(p->attacker_id) && g_c_monsters.count(p->victim_id)) {
				if ((nullptr != g_c_skills[p->attacker_id]) && (nullptr != g_c_monsters[p->victim_id])) {
					g_c_skills[p->attacker_id]->Overlap(g_c_monsters[p->victim_id]);
					Cast<AEnemyCharacter>(g_c_monsters[p->victim_id])->Overlap(g_c_skills[p->attacker_id]);
				}
			}
			break;

		case SKILL_PLAYER_COLLISION:
			if (g_c_skills.count(p->attacker_id) && g_c_players[p->victim_id]) {
				if (nullptr != g_c_skills[p->attacker_id]) g_c_skills[p->attacker_id]->Overlap(g_c_monsters[p->victim_id]);
			}
			break;
		}
	}

	case H2C_SKILL_CREATE_PACKET: {
		skill_create_packet* p = reinterpret_cast<skill_create_packet*>(packet);
		switch (p->skill_type) {
		case SKILL_WIND_FIRE_BOMB:
			if (g_c_skills.count(p->old_skill_id)) {
				if (nullptr != g_c_skills[p->old_skill_id]) { break; }

				if (g_c_skills[p->old_skill_id]->IsA(AMyWindCutter::StaticClass())) {
					Cast<AMyWindCutter>(g_c_skills[p->old_skill_id])->MixBombAttack(EClassType::CT_Fire, p->new_skill_id);
				}
			}
			break;

		case SKILL_WIND_ICE_BOMB:
			if (g_c_skills.count(p->old_skill_id)) {
				if (nullptr != g_c_skills[p->old_skill_id]) { break; }

				if (g_c_skills[p->old_skill_id]->IsA(AMyWindCutter::StaticClass())) {
					Cast<AMyWindCutter>(g_c_skills[p->old_skill_id])->MixBombAttack(EClassType::CT_Ice, p->new_skill_id);
				}
			}
			break;

		case SKILL_WIND_WIND_TORNADO:
			if (g_c_skills.count(p->old_skill_id)) {
				if (nullptr != g_c_skills[p->old_skill_id]) { break; }

				if (g_c_skills[p->old_skill_id]->IsA(AMyWindSkill::StaticClass())) {
					Cast<AMyWindSkill>(g_c_skills[p->old_skill_id])->SpawnMixTonado(p->new_skill_id);
				}
			}
			break;
		}
		break;
	}

	case H2C_INIT_MONSTER_PACKET: {
		hc_init_monster_packet* p = reinterpret_cast<hc_init_monster_packet*>(packet);

		unsigned short monster_count = p->count;

		for (unsigned short i = 0; i < monster_count; ++i) {
			monster_init_info info = p->monsters[i];

			AsyncTask(ENamedThreads::GameThread, [info]() {
				UWorld* World = GEngine->GetWorldFromContextObjectChecked(GEngine->GameViewport);
				if (!World) return;

				FVector SpawnLocation(info.x, info.y, info.z);
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

				NewMonster->set_id(info.id);
				NewMonster->set_hp(info.hp);
				NewMonster->SetActorLocation(FVector(info.x, info.y, info.z));
				NewMonster->set_target_location(FVector(info.target_x, info.target_y, info.target_z));

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

				g_c_monsters[info.id] = NewMonster;

				if (info.hp <= 0.0f) {
					Cast<AEnemyCharacter>(g_c_monsters[info.id])->Die();
				}
			});
		}
		break;
	}

	case H2C_MONSTER_MOVE_PACKET: {
		hc_monster_move_packet* p = reinterpret_cast<hc_monster_move_packet*>(packet);

		if (g_c_monsters.count(p->id)) {
			if (nullptr == g_c_monsters[p->id]) { break; }

			AEnemyCharacter* Monster = Cast<AEnemyCharacter>(g_c_monsters[p->id]);

			Monster->set_target_location(FVector(p->target_x, p->target_y, p->target_z));
		}
		break;
	}

	case H2C_MONSTER_ATTACK_PACKET: {
		hc_monster_attack_packet* p = reinterpret_cast<hc_monster_attack_packet*>(packet);

		if (g_c_monsters.count(p->id)) {
			if (nullptr == g_c_monsters[p->id]) { break; }

			Cast<AEnemyCharacter>(g_c_monsters[p->id])->MeleeAttack();
		}

		break;
	}

	case H2C_MONSTER_RESPAWN_PACKET: {
		hc_monster_respawn_packet* p = reinterpret_cast<hc_monster_respawn_packet*>(packet);

		if (g_c_monsters.count(p->id)) {
			if (nullptr == g_c_monsters[p->id]) { break; }

			Cast<AEnemyCharacter>(g_c_monsters[p->id])->Respawn(FVector(p->respawn_x, p->respawn_y, p->respawn_z));
		}

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
	char* p = g_c_recv_over.m_buffer;
	unsigned char packet_size = p[0];
	int remained = g_c_remained + num_bytes;

	while (packet_size <= remained) {
		c_process_packet(p);
		p += packet_size;
		remained -= packet_size;
		if (!remained) {
			break;
		}
		packet_size = p[0];
	}

	g_c_remained = remained;
	if (remained) {
		memcpy(g_c_recv_over.m_buffer, p, remained);
	}

	DWORD recv_bytes;
	DWORD recv_flag = 0;
	g_c_recv_over.m_wsabuf[0].buf = g_c_recv_over.m_buffer + g_c_remained;
	g_c_recv_over.m_wsabuf[0].len = sizeof(g_c_recv_over.m_buffer) - g_c_remained;
	WSARecv(g_c_socket, g_c_recv_over.m_wsabuf, 1, &recv_bytes, &recv_flag, &g_c_recv_over.m_over, c_recv_callback);
}

void CALLBACK c_send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED p_over, DWORD flags) {
	EXP_OVER* p = reinterpret_cast<EXP_OVER*>(p_over);
	delete p;
}
