// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_Return.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

#include "EnemyCharacter.h"

UBTTask_Return::UBTTask_Return() {
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Return::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn) { return EBTNodeResult::Failed; }

    Cast<AEnemyCharacter>(Pawn)->StartHeal();

    StartLocation = OwnerComp.GetBlackboardComponent()->GetValueAsVector(TEXT("StartLocation"));

    return EBTNodeResult::InProgress;
}

void UBTTask_Return::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn) { return; }

    FVector current_location = Pawn->GetActorLocation();
    FVector to_target = StartLocation - current_location;

    float dist = to_target.Size2D();

    if (dist < 50.0f) {
        Cast<AEnemyCharacter>(Pawn)->StopHeal();

        OwnerComp.GetBlackboardComponent()->ClearValue(TEXT("bIsReturning"));

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
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