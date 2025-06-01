// Fill out your copyright notice in the Description page of Project Settings.


#include "MMBossStoneThrowAnimNotify.h"
#include "ImpactPointInterface.h"
#include "MidBossEnemyCharacter.h"
#include "MyStoneSkill.h"
#include "SESSION.h"

void UMMBossStoneThrowAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (!MeshComp) return;

    AActor* OwnerActor = MeshComp->GetOwner();
    IImpactPointInterface* ImpactPointOwner = Cast<IImpactPointInterface>(OwnerActor);
    AMidBossEnemyCharacter* BossCharacter = Cast<AMidBossEnemyCharacter>(OwnerActor);
    if (!BossCharacter || !BossCharacter->bIsPlayingMontageSection)
    {
        // 충돌 등으로 인해 몽타주가 중단되었거나 실행 중이 아님
        return;
    }
    if (ImpactPointOwner && BossCharacter && BossCharacter->GetStoneSkillClass())
    {
        
        FVector TargetLocation = ImpactPointOwner->GetCurrentImpactPoint(); // 임팩트 위치 사용

        // 소켓 위치에서 스폰
        FVector SpawnLocation = MeshComp->GetSocketLocation(BossCharacter->BaseAttackSocketName);
        FVector FireDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
        FRotator FireRotation = FireDirection.Rotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerActor;
        SpawnParams.Instigator = OwnerActor->GetInstigator();

        AMyStoneSkill* StoneSkill = MeshComp->GetWorld()->SpawnActor<AMyStoneSkill>(BossCharacter->GetStoneSkillClass(), SpawnLocation, FireRotation, SpawnParams);

        if (StoneSkill)
        {
            unsigned short skill_id = Cast<AMidBossEnemyCharacter>(OwnerActor)->get_skill_id();

            StoneSkill->SetID(skill_id);

            g_c_skills.emplace(skill_id, StoneSkill);

            if (g_c_skill_collisions.count(skill_id)) {
                while (!g_c_skill_collisions[skill_id].empty()) {
                    char skill_type = g_c_skill_collisions[skill_id].front();
                    g_c_skill_collisions[skill_id].pop();

                    g_c_skills[skill_id]->Overlap(skill_type);
                }
            }

            if (g_c_object_collisions.count(skill_id)) {
                while (!g_c_object_collisions[skill_id].empty()) {
                    unsigned short object_id = g_c_object_collisions[skill_id].front();
                    g_c_object_collisions[skill_id].pop();

                    g_c_skills[skill_id]->Overlap(object_id);
                }
            }

            StoneSkill->Fire(TargetLocation); // 방향 계산은 내부에서 자동 처리
        }
    }
}