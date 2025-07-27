// Fill out your copyright notice in the Description page of Project Settings.


#include "MMBossIceWallAnimNotify.h"
#include "MidBossEnemyCharacter.h"
#include "MyIceSkill.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "SESSION.h"

void UMMBossIceWallAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (!MeshComp) return;

    AActor* OwnerActor = MeshComp->GetOwner();
    AMidBossEnemyCharacter* BossCharacter = Cast<AMidBossEnemyCharacter>(OwnerActor);

    if (BossCharacter && BossCharacter->GetIceSkillClass())
    {
        FVector SpawnLoc = BossCharacter->GetMesh()->GetSocketLocation(BossCharacter->BaseAttackSocketName);

        unsigned short skill_id = Cast<AMidBossEnemyCharacter>(OwnerActor)->get_skill_id();

        
        FRotator SpawnRot = BossCharacter->GetCurrentImpactRot();
        FActorSpawnParameters Params;
        Params.Owner = OwnerActor;
        Params.Instigator = OwnerActor->GetInstigator();

        AMyIceSkill* IceSkill = MeshComp->GetWorld()->SpawnActor<AMyIceSkill>(BossCharacter->GetWindSkillClass(), SpawnLoc, SpawnRot, Params);

        if (IceSkill)
        {
            IceSkill->SetID(skill_id);

            g_c_skills.emplace(skill_id, IceSkill);

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

            IceSkill->SpawnIceSkill(SpawnLoc,SpawnRot);

            ++skill_id;
        }
        
    }
}