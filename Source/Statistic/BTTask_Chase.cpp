// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_Chase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

#include "SESSION.h"
#include "PlayerCharacter.h"
#include "EnemyCharacter.h"

UBTTask_Chase::UBTTask_Chase() {
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Chase::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
    return EBTNodeResult::InProgress;
}

void UBTTask_Chase::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds){
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn) { return; }

    FVector current_location = Pawn->GetActorLocation();
    FVector target_location = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("TargetLocation"));
    FVector to_target = target_location - current_location;

    float dist = to_target.Size2D();

    if (dist < 100.0f) {
        Cast<AEnemyCharacter>(Pawn)->MeleeAttack();

        OwnerComp.GetBlackboardComponent()->ClearValue(TEXT("TargetLocation"));

        {
            MonsterEvent monster_event = AttackEvent(Cast<AEnemyCharacter>(Pawn)->get_id());
            std::lock_guard<std::mutex> lock(g_s_monster_events_l);
            g_s_monster_events.push(monster_event);
        }

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    } 

    FVector start_location = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("StartLocation"));

    dist = (current_location - start_location).Size2D();

    if (dist > 2000.0f) {
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("bIsReturning"), true);

        {
            MonsterEvent monster_event = TargetEvent(Cast<AEnemyCharacter>(Pawn)->get_id(), start_location);
            std::lock_guard<std::mutex> lock(g_s_monster_events_l);
            g_s_monster_events.push(monster_event);
        }

        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector direction = to_target.GetSafeNormal();

    // Rotate
    FRotator target_rotation = direction.Rotation();
    FRotator current_rotation = Pawn->GetActorRotation();

    FRotator new_rotation = FMath::RInterpTo(current_rotation, target_rotation, GetWorld()->GetDeltaSeconds(), 5.0f);
    Pawn->SetActorRotation(new_rotation);

    // Move
    Pawn->AddMovementInput(direction);
}
