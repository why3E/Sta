// Fill out your copyright notice in the Description page of Project Settings.


#include "MMSpawnStoneWave.h"
#include "MyStoneWeapon.h"
#include "MyStoneWave.h"
#include "AnimationWeaponInterface.h"
#include "ImpactPointInterface.h"


void UMMSpawnStoneWave::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
                AMyStoneWeapon* Weapon = Cast<AMyStoneWeapon>(WeaponPawn->GetWeapon());
                if (Weapon)
                {
                    UE_LOG(LogTemp, Warning, TEXT("WeaponPawn is Spawn"));
                    Weapon->SpawnStoneWave(FireLocation); // 에너지볼 스폰 함수 호출
                }
            }
        }
    }
}