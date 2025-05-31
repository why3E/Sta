#include "MMBossWindLaserFireAnimNotify.h"
#include "MidBossEnemyCharacter.h"
#include "MyWindLaser.h"

void UMMBossWindLaserFireAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if (!MeshComp) return;

    AMidBossEnemyCharacter* BossCharacter = Cast<AMidBossEnemyCharacter>(MeshComp->GetOwner());
    if (!BossCharacter) return;
    if (!BossCharacter || !BossCharacter->bIsPlayingMontageSection)
    {
        // 충돌 등으로 인해 몽타주가 중단되었거나 실행 중이 아님
        return;
    }
    // 보관된 WindLaser가 있으면 발사
    if (BossCharacter->CurrentWindLaser)
    {
        BossCharacter->CurrentWindLaser->SpawnFiringLaser();
        BossCharacter->CurrentWindLaser = nullptr; // 사용 후 해제
    }
}