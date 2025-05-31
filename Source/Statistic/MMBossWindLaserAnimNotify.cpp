#include "MMBossWindLaserAnimNotify.h"
#include "MidBossEnemyCharacter.h"
#include "MyWindLaser.h"
#include "ImpactPointInterface.h"

void UMMBossWindLaserAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
    // WindLaser 사용
    if (ImpactPointOwner && BossCharacter && BossCharacter->GetWindLaserClass())
    {
        FVector TargetLocation = ImpactPointOwner->GetFireLocation();

        // 소켓 위치에서 스폰
        FVector SpawnLocation = MeshComp->GetSocketLocation(BossCharacter->LaserAttackSocketName);
        FVector FireDirection = (TargetLocation - SpawnLocation).GetSafeNormal();
        FRotator FireRotation = FireDirection.Rotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = OwnerActor;
        SpawnParams.Instigator = OwnerActor->GetInstigator();

        AMyWindLaser* WindLaser = MeshComp->GetWorld()->SpawnActor<AMyWindLaser>(
            BossCharacter->GetWindLaserClass(), SpawnLocation, FireRotation, SpawnParams);

        if (WindLaser)
        {
            WindLaser->SpawnChargingLaser(); // 차징만 시작
            BossCharacter->CurrentWindLaser = WindLaser; // 보스가 임시로 보관
        }
    }
}