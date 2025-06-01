#include "BTTask_Attack.h"
#include "SESSION.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_Attack::UBTTask_Attack() {
    bNotifyTaskFinished = true;
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
    Super::ExecuteTask(OwnerComp, NodeMemory);

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn) { return EBTNodeResult::Failed; }

    EnemyCharacter = Cast<AEnemyCharacter>(Pawn);
    CachedOwnerComp = &OwnerComp;

    if (EnemyCharacter == nullptr) { return EBTNodeResult::Failed; }
    
    EnemyCharacter->OnAttackEnded.AddUniqueDynamic(this, &UBTTask_Attack::OnAttackEnded);

    {
        MonsterEvent monster_event = AttackEvent(EnemyCharacter->get_id(), EnemyCharacter->GetActorLocation(), AttackType::Melee);
        std::lock_guard<std::mutex> lock(g_s_monster_events_l);
        g_s_monster_events.push(monster_event);
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_Attack::OnAttackEnded() {
    CachedOwnerComp->GetBlackboardComponent()->ClearValue(TEXT("bIsAttacking"));

    if (EnemyCharacter) {
        EnemyCharacter->OnAttackEnded.RemoveDynamic(this, &UBTTask_Attack::OnAttackEnded);
    }

    if (CachedOwnerComp) {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTTask_Attack::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) {
    if (EnemyCharacter) {
        EnemyCharacter->OnAttackEnded.RemoveDynamic(this, &UBTTask_Attack::OnAttackEnded);
    }

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}