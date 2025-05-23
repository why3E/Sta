// Fill out your copyright notice in the Description page of Project Settings.

#include "AIController.h"
#include "EnemyCharacter.h"
#include "PlayerCharacter.h"
#include "EnemyAIController.h"
#include "BTService_PlayerLocationSeen.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

#include "SESSION.h"

UBTService_PlayerLocationSeen::UBTService_PlayerLocationSeen()
{
    NodeName = "Update Player Location If Seen";
}
void UBTService_PlayerLocationSeen::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    if (g_is_host) {
        AAIController* AIController = Cast<AAIController>(OwnerComp.GetAIOwner());
        if (!AIController) { return; }

        AEnemyCharacter* Monster = Cast<AEnemyCharacter>(AIController->GetPawn());
        if (!Monster || Monster->get_is_attacking()) { return; }

        UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();
        if (!BlackboardComp) { return; }

        bool found_target = false;

        for (char client_id = 0; client_id < MAX_CLIENTS; ++client_id) {
            APlayerCharacter* player = g_c_players[client_id];

            if (!player) continue;

            FVector player_location = player->GetActorLocation();
            FVector monster_location = Monster->GetActorLocation();

            float dist = FVector::Dist2D(player_location, monster_location);

            if (dist < 3000.0f && AIController->LineOfSightTo(player)) {
                found_target = true;
                BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), player->GetActorLocation());
                break;
            }
        }

        if (!found_target) {
            BlackboardComp->ClearValue(GetSelectedBlackboardKey());
        }
    }
}
