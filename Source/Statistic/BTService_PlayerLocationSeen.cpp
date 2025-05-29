// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_PlayerLocationSeen.h"

#include "SESSION.h"
#include "AIController.h"
#include "EnemyCharacter.h"
#include "PlayerCharacter.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

UBTService_PlayerLocationSeen::UBTService_PlayerLocationSeen()
{
    NodeName = "Update Player Location If Seen";
}

void UBTService_PlayerLocationSeen::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
    if (!AIController) { return; }

    AEnemyCharacter* Monster = Cast<AEnemyCharacter>(AIController->GetPawn());
    if (!Monster || Monster->get_is_attacking()) { return; }

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
    if (!BlackboardComp) { return; }

    char c_id = 0;
    float min_dist = 2000.0f;
    bool found_target = false;

    for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
        APlayerCharacter* player = g_c_players[client_id];

        if (!player) continue;

        FVector player_location = player->GetActorLocation();
        FVector monster_location = Monster->GetActorLocation();

        float dist = FVector::Dist2D(player_location, monster_location);

        if (min_dist > dist) {
            c_id = client_id;
            min_dist = dist;
            found_target = true;
        }
    }

    if (found_target) {
        APlayerCharacter* player = g_c_players[c_id];

        if (AIController->LineOfSightTo(player)) {
            BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), player->GetActorLocation());

            {
                MonsterEvent monster_event = TargetEvent(Monster->get_id(), player->GetActorLocation());
                std::lock_guard<std::mutex> lock(g_s_monster_events_l);
                g_s_monster_events.push(monster_event);
            }
        }
    } else {
        BlackboardComp->ClearValue(GetSelectedBlackboardKey());
    }
}
