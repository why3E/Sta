// Fill out your copyright notice in the Description page of Project Settings.


#include "MMShootFireBall.h"
#include "AnimationWeaponInterface.h"
#include "MyFireWeapon.h"
#include "MyFireBall.h"

void UMMShootFireBall::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if(MeshComp)
    {
        IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
        if (WeaponPawn)
        {
            AMyFireWeapon* Weapon = Cast<AMyFireWeapon>(WeaponPawn->GetWeapon());
            if (Weapon)
            {
                Weapon->ShootFireBall(); // 에너지볼 스폰 함수 호출
            }
        }
    }
}