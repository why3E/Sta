// Fill out your copyright notice in the Description page of Project Settings.


#include "MMSpawnFireBall.h"
#include "AnimationWeaponInterface.h"
#include "ImpactPointInterface.h"
#include "MyFireWeapon.h"
#include "MyFireBall.h"

void UMMSpawnFireBall::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
                UE_LOG(LogTemp, Warning, TEXT("WeaponPawn is valid"));
                AMyFireWeapon* Weapon = Cast<AMyFireWeapon>(WeaponPawn->GetWeapon());
                if (Weapon)
                {
                    UE_LOG(LogTemp, Warning, TEXT("WeaponPawn is Spawn"));
                    Weapon->SpawnFireBall(FireLocation); // 에너지볼 스폰 함수 호출
                }
            }
        }
    }
}