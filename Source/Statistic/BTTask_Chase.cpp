// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_Chase.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

#include "SESSION.h"
#include "MyEnemyBase.h"
#include "PlayerCharacter.h"

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

    if (dist < Cast<AMyEnemyBase>(Pawn)->get_attack_radius()) {
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("bIsAttacking"), true);

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    } 

    FVector start_location = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("StartLocation"));

    dist = (current_location - start_location).Size2D();

    if (dist > Cast<AMyEnemyBase>(Pawn)->m_track_radius) {
        OwnerComp.GetBlackboardComponent()->SetValueAsBool(TEXT("bIsReturning"), true);

        {
            MonsterEvent monster_event = TargetEvent(Cast<AMyEnemyBase>(Pawn)->get_id(), start_location);
            std::lock_guard<std::mutex> lock(g_s_monster_events_l);
            g_s_monster_events.push(monster_event);
        }

        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector direction = to_target.GetSafeNormal();

    // Rotate
    FRotator target_rotation = FRotator(0.0f, direction.Rotation().Yaw, 0.0f);
    FRotator current_rotation = Pawn->GetActorRotation();

    FRotator new_rotation = FMath::RInterpTo(current_rotation, target_rotation, GetWorld()->GetDeltaSeconds(), 5.0f);
    Pawn->SetActorRotation(new_rotation);

    // Move
    Pawn->AddMovementInput(direction);
}
