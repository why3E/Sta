// Fill out your copyright notice in the Description page of Project Settings.
#include "MMBossStoneAttackAnimNotify.h"
#include "ImpactPointInterface.h"
#include "MidBossEnemyCharacter.h"
#include "MyStoneWave.h"
#include "Kismet/GameplayStatics.h"

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
        FVector SpawnLocation = MyLocation + FireDirection * 300.f;
        FRotator FireRotation = FireDirection.Rotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerActor;
        SpawnParams.Instigator = OwnerActor->GetInstigator();

        AMyStoneWave* StoneWave = MeshComp->GetWorld()->SpawnActor<AMyStoneWave>(
            BossCharacter->GetStoneWaveClass(), SpawnLocation, FireRotation, SpawnParams);

        if (StoneWave)
        {
            StoneWave->Fire(TargetLocation);
        }
    }
}
