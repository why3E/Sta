// Fill out your copyright notice in the Description page of Project Settings.
#include "MMBossStoneAttackAnimNotify.h"
#include "ImpactPointInterface.h"
#include "MidBossEnemyCharacter.h"
#include "MyStoneWave.h"
#include "Kismet/GameplayStatics.h"
#include "SESSION.h"

void UMMBossStoneAttackAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

    // 정상 실행 시에만 발사
    if (ImpactPointOwner && BossCharacter->GetStoneWaveClass())
    {
        FVector TargetLocation = ImpactPointOwner->GetFireLocation(); // 타겟 위치
        FVector MyLocation = OwnerActor->GetActorLocation();

        FVector FireDirection = (TargetLocation - MyLocation).GetSafeNormal();
        FVector SpawnLocation = MyLocation + FireDirection * 600.f;
        FRotator FireRotation = FireDirection.Rotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerActor;
        SpawnParams.Instigator = OwnerActor->GetInstigator();

        AMyStoneWave* StoneWave = MeshComp->GetWorld()->SpawnActor<AMyStoneWave>(BossCharacter->GetStoneWaveClass(), SpawnLocation, FireRotation, SpawnParams);

        if (StoneWave)
        {
            unsigned short skill_id = Cast<AMidBossEnemyCharacter>(OwnerActor)->get_skill_id();

            StoneWave->SetID(skill_id);

            g_c_skills.emplace(skill_id, StoneWave);

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

            StoneWave->Fire(TargetLocation);
        }
    }
}
