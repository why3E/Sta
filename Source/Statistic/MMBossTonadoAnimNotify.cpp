#include "MMBossTonadoAnimNotify.h"
#include "MidBossEnemyCharacter.h"
#include "MyWindSkill.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

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

            AMyWindSkill* WindSkill = MeshComp->GetWorld()->SpawnActor<AMyWindSkill>(
                BossCharacter->GetWindSkillClass(), SpawnLoc, SpawnRot, Params);
            if (WindSkill)
            {
                WindSkill->SpawnWindTonado(SpawnLoc);
            }
        }
    }
}