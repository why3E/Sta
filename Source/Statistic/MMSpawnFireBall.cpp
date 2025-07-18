// Fill out your copyright notice in the Description page of Project Settings.


#include "MMSpawnFireBall.h"
#include "AnimationWeaponInterface.h"
#include "ImpactPointInterface.h"
#include "MyFireWeapon.h"
#include "PlayerCharacter.h"
#include "MyFireBall.h"

void UMMSpawnFireBall::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if(MeshComp)
    {
        IImpactPointInterface* ImpactPointOwner = Cast<IImpactPointInterface>(MeshComp->GetOwner());
        APlayerCharacter* PlayerCh = Cast<APlayerCharacter>(MeshComp->GetOwner());
        if (ImpactPointOwner)
        {
            FVector FireLocation = ImpactPointOwner->GetFireLocation();

            IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
            if (WeaponPawn)
            {
                AMyFireWeapon* Weapon = Cast<AMyFireWeapon>(WeaponPawn->GetWeapon());
                if (Weapon)
                {
                    PlayerCh->bIsAttacking = true; // 공격 상태 해제
                    Weapon->SpawnFireBall(FireLocation); // 에너지볼 스폰 함수 호출
                }
            }
        }
    }
}