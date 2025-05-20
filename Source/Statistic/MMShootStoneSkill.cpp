// Fill out your copyright notice in the Description page of Project Settings.


#include "MMShootStoneSkill.h"
#include "AnimationWeaponInterface.h"
#include "MyStoneWeapon.h"
#include "MyStoneSkill.h"

void UMMShootStoneSkill::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if(MeshComp)
    {
        IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
        if (WeaponPawn)
        {
            AMyStoneWeapon* Weapon = Cast<AMyStoneWeapon>(WeaponPawn->GetWeapon());
            if (Weapon)
            {
                UE_LOG(LogTemp, Warning, TEXT("Impact Point: %s"), *Weapon->GetActorLocation().ToString());
                Weapon->ShootStoneSkill(); // 에너지볼 스폰 함수 호출
            }
        }
    }
}