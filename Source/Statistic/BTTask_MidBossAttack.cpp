// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MidBossAttack.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"

UBTTask_MidBossAttack::UBTTask_MidBossAttack() {
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_MidBossAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;

    if (!Pawn) { return EBTNodeResult::Failed; }

    MidBossEnemyCharacter = Cast<AMidBossEnemyCharacter>(Pawn);
    CachedOwnerComp = &OwnerComp;

    if (MidBossEnemyCharacter == nullptr) { return EBTNodeResult::Failed; }

    MidBossEnemyCharacter->OnAttackEnded.AddUniqueDynamic(this, &UBTTask_MidBossAttack::OnAttackEnded);

    MidBossEnemyCharacter->FindPlayerCharacter();

    if (MidBossEnemyCharacter->CachedPlayerCharacter) {
        AttackType skill_type = static_cast<AttackType>((uid(dre) % 5) + 1);
        FVector skill_location = MidBossEnemyCharacter->CachedPlayerCharacter->GetActorLocation();

        {
            MonsterEvent monster_event = SkillEvent(MidBossEnemyCharacter->get_id(), MidBossEnemyCharacter->GetActorLocation(), skill_type, skill_location);
            std::lock_guard<std::mutex> lock(g_s_monster_events_l);
            g_s_monster_events.push(monster_event);
        }

        return EBTNodeResult::InProgress;
    } else {
        return EBTNodeResult::Failed;
    }
}

void UBTTask_MidBossAttack::OnAttackEnded() {
    CachedOwnerComp->GetBlackboardComponent()->ClearValue(TEXT("bIsAttacking"));

    if (MidBossEnemyCharacter) {
        MidBossEnemyCharacter->OnAttackEnded.RemoveDynamic(this, &UBTTask_MidBossAttack::OnAttackEnded);
    }

    if (CachedOwnerComp) {
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}

