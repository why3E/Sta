// Fill out your copyright notice in the Description page of Project Settings.


#include "MMSpawnWindCutter.h"
#include "AnimationWeaponInterface.h"
#include "MyWindWeapon.h"
#include "MyWindCutter.h"

void UMMSpawnWindCutter::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);
    if(MeshComp)
    {
        IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
        if (WeaponPawn)
        {
            AMyWindWeapon* Weapon = Cast<AMyWindWeapon>(WeaponPawn->GetWeapon());
            if (Weapon)
            {
                Weapon->SpawnWindCutter(); // 에너지볼 스폰 함수 호출
            }
        }
    }
}