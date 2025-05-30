// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_Wander.h"
#include "SESSION.h"
#include "EnemyCharacter.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Wander::UBTTask_Wander() {
    bNotifyTick = true; 
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Wander::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return EBTNodeResult::Failed;

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) return EBTNodeResult::Failed;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return EBTNodeResult::Failed;

    FVector Origin = BlackboardComp->GetValueAsVector(TEXT("StartLocation"));

    // Create Random Vector
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; 
    RandomDirection.Normalize();

    // Calculate Destination
    FVector Destination = Origin + RandomDirection * SearchRadius;
    BlackboardComp->SetValueAsVector(TEXT("WanderLocation"), Destination);

    {
        MonsterEvent monster_event = TargetEvent(Cast<AEnemyCharacter>(AIPawn)->get_id(), Destination);
        std::lock_guard<std::mutex> lock(g_s_monster_events_l);
        g_s_monster_events.push(monster_event);
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_Wander::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    APawn* AIPawn = AIController->GetPawn();
    if (!AIPawn) {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector Destination = BlackboardComp->GetValueAsVector(TEXT("WanderLocation"));
    FVector CurrentLocation = AIPawn->GetActorLocation();

    FVector Direction = (Destination - CurrentLocation).GetSafeNormal2D();

    // Rotate
    FRotator TargetRotation = Direction.Rotation();
    FRotator CurrentRotation = AIPawn->GetActorRotation();

    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 5.0f);
    AIPawn->SetActorRotation(NewRotation);

    // Move
    AIPawn->AddMovementInput(Direction, 1.0f);

    // Finish Task
    float Distance = FVector::Dist2D(CurrentLocation, Destination);
    if (Distance < 100.0f) { 
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}
