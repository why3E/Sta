#include "EnemyAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
void AEnemyAIController::BeginPlay()
{
    Super::BeginPlay();
    
    if (AIBehavior != nullptr)
    {
        RunBehaviorTree(AIBehavior);

        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        GetBlackboardComponent()->SetValueAsVector(TEXT("StartLocation"), GetPawn()->GetActorLocation());

        GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), PlayerPawn);
    }
}


void AEnemyAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
   
}
