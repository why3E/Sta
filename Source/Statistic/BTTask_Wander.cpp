// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_Wander.h"
#include "SESSION.h"
#include "MyEnemyBase.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Wander::UBTTask_Wander() {
    bNotifyTick = true; 
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn) { return EBTNodeResult::Failed; }

    FVector Origin = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("StartLocation"));

    // Create Random Vector
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; 
    RandomDirection.Normalize();

    // Calculate Destination
    FVector Destination = Origin + RandomDirection * Cast<AMyEnemyBase>(Pawn)->m_wander_radius;
    OwnerComp.GetBlackboardComponent()->SetValueAsVector(TEXT("WanderLocation"), Destination);

    {
        MonsterEvent monster_event = TargetEvent(Cast<AMyEnemyBase>(Pawn)->get_id(), Destination);
        std::lock_guard<std::mutex> lock(g_s_monster_events_l);
        g_s_monster_events.push(monster_event);
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_Wander::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn) { return; }

    FVector Destination = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("WanderLocation"));
    FVector CurrentLocation = Pawn->GetActorLocation();

    FVector Direction = (Destination - CurrentLocation).GetSafeNormal2D();

    // Rotate
    FRotator TargetRotation = FRotator(0.0f, Direction.Rotation().Yaw, 0.0f);
    FRotator CurrentRotation = Pawn->GetActorRotation();

    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 5.0f);
    Pawn->SetActorRotation(NewRotation);

    // Move
    Pawn->AddMovementInput(Direction, 1.0f);

    // Finish Task
    if ((CurrentLocation - Destination).Size2D() < 100.0f) {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
