#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();
    
    if (AIBehavior != nullptr) {
        RunBehaviorTree(AIBehavior);

        GetBlackboardComponent()->SetValueAsVector(TEXT("StartLocation"), GetPawn()->GetActorLocation());
    }
}

void AEnemyAIController::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);   
}
