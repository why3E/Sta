// Fill out your copyright notice in the Description page of Project Settings.


#include "MMShootFireBall.h"
#include "AnimationWeaponInterface.h"
#include "MyFireWeapon.h"
#include "PlayerCharacter.h"
#include "MyFireBall.h"

void UMMShootFireBall::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if(MeshComp)
    {
        IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
        APlayerCharacter* PlayerCh = Cast<APlayerCharacter>(MeshComp->GetOwner());
        if (WeaponPawn)
        {
            AMyFireWeapon* Weapon = Cast<AMyFireWeapon>(WeaponPawn->GetWeapon());
            if (Weapon)
            {
                PlayerCh->bIsAttacking = false; // 공격 상태 해제
                Weapon->ShootFireBall(); // 에너지볼 스폰 함수 호출
            }
        }
    }
}