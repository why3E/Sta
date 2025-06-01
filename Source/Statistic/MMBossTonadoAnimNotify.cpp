#include "MMBossTonadoAnimNotify.h"
#include "MidBossEnemyCharacter.h"
#include "MyWindSkill.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "SESSION.h"

void UMMBossTonadoAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (!MeshComp) return;

    AActor* OwnerActor = MeshComp->GetOwner();
    AMidBossEnemyCharacter* BossCharacter = Cast<AMidBossEnemyCharacter>(OwnerActor);

    if (!BossCharacter || !BossCharacter->bIsPlayingMontageSection)
    {
        // 충돌 등으로 인해 몽타주가 중단되었거나 실행 중이 아님
        return;
    }

    if (BossCharacter && BossCharacter->GetWindSkillClass())
    {
        TArray<FVector> SpawnLocations = BossCharacter->GenerateWindTonadoLocations(3, 500.f, 1000.f, 300.f);

        for (const FVector& SpawnLoc : SpawnLocations)
        {
            FRotator SpawnRot = FRotator::ZeroRotator;
            FActorSpawnParameters Params;
            Params.Owner = OwnerActor;
            Params.Instigator = OwnerActor->GetInstigator();

            AMyWindSkill* WindSkill = MeshComp->GetWorld()->SpawnActor<AMyWindSkill>(BossCharacter->GetWindSkillClass(), SpawnLoc, SpawnRot, Params);

            if (WindSkill)
            {
                unsigned short skill_id = Cast<APlayerCharacter>(OwnerActor)->get_skill_id();

                WindSkill->SetID(skill_id);

                g_c_skills.emplace(skill_id, WindSkill);

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

                WindSkill->SpawnWindTonado(SpawnLoc);
            }
        }
    }
}