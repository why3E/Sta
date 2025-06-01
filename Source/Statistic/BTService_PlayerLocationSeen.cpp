// Fill out your copyright notice in the Description page of Project Settings.

#include "BTService_PlayerLocationSeen.h"
#include "SESSION.h"
#include "AIController.h"
#include "MyEnemyBase.h"
#include "PlayerCharacter.h"
#include "EnemyAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

UBTService_PlayerLocationSeen::UBTService_PlayerLocationSeen() {

}

void UBTService_PlayerLocationSeen::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
    if (OwnerComp.GetBlackboardComponent()->GetValueAsBool(TEXT("bIsReturning")) ||
        OwnerComp.GetBlackboardComponent()->GetValueAsBool(TEXT("bIsAttacking")) ||
        OwnerComp.GetBlackboardComponent()->GetValueAsBool(TEXT("bIsStunned"))) {
        return; 
    }

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn) { return; }

    AMyEnemyBase* monster = Cast<AMyEnemyBase>(AICon->GetPawn());

    if (!monster) { return; }

    FVector monster_location = monster->GetActorLocation();

    float min_dist = monster->m_view_radius;
    FVector target_location = FVector::ZeroVector;
    bool found = false;

    for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
        APlayerCharacter* player = g_c_players[client_id];

        if (!player) { continue; }

        FVector player_location = player->GetActorLocation();

        float dist = (monster_location - player_location).Size2D();

        if (dist < min_dist) {
            min_dist = dist;
            target_location = player_location;
            found = true;
        }
    }

    if (found) {
        OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), target_location);

        if (!OwnerComp.GetBlackboardComponent()->GetValueAsBool(TEXT("bIsReturning"))) {
            MonsterEvent monster_event = TargetEvent(Cast<AMyEnemyBase>(Pawn)->get_id(), target_location);
            std::lock_guard<std::mutex> lock(g_s_monster_events_l);
            g_s_monster_events.push(monster_event);
        }
    } else {
        OwnerComp.GetBlackboardComponent()->ClearValue(TEXT("TargetLocation"));
    }

    return;
}
