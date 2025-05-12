#include "MMWalkSound.h"
#include "PlayerCharacter.h" // APlayerCharacter 헤더 포함
#include "Kismet/GameplayStatics.h"

void UMMWalkSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    // 캐릭터가 APlayerCharacter인지 확인
    if (MeshComp && MeshComp->GetOwner())
    {
        APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(MeshComp->GetOwner());
        if (PlayerCharacter)
        {
            // 발자국 효과음 재생
            PlayerCharacter->PlayFootstepSound();
        }
    }
}