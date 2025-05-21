// Fill out your copyright notice in the Description page of Project Settings.


#include "MMShootIceArrow.h"
#include "AnimationWeaponInterface.h"
#include "ImpactPointInterface.h"
#include "MyIceWeapon.h"


void UMMShootIceArrow::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if(MeshComp)
    {
        IImpactPointInterface* ImpactPointOwner = Cast<IImpactPointInterface>(MeshComp->GetOwner());
        if (ImpactPointOwner)
        {
            FVector FireLocation = ImpactPointOwner->GetFireLocation();

            IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
            if (WeaponPawn)
            {
                AMyIceWeapon* Weapon = Cast<AMyIceWeapon>(WeaponPawn->GetWeapon());
                if (Weapon)
                {
                    Weapon->ShootIceArrow(FireLocation); // 에너지볼 스폰 함수 호출
                }
            }
        }
    }
}