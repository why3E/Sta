// Fill out your copyright notice in the Description page of Project Settings.


#include "MMSpawnStoneSkill.h"
#include "AnimationWeaponInterface.h"
#include "ImpactPointInterface.h"
#include "MyStoneWeapon.h"
#include "MyStoneSkill.h"

void UMMSpawnStoneSkill::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (MeshComp)
    {
        // MeshComp의 소유자를 ImpactPointInterface로 캐스팅
        IImpactPointInterface* ImpactPointOwner = Cast<IImpactPointInterface>(MeshComp->GetOwner());
        if (ImpactPointOwner)
        {
            // GetCurrentImpactPoint 호출하여 위치 가져오기
            FVector ImpactPoint = ImpactPointOwner->GetCurrentImpactPoint();
            
            UE_LOG(LogTemp, Warning, TEXT("Impactwd Point: %s"), *ImpactPoint.ToString());

            // 무기 인터페이스를 통해
            IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
            if (WeaponPawn)
            {
                AMyStoneWeapon* Weapon = Cast<AMyStoneWeapon>(WeaponPawn->GetWeapon());
                if (Weapon)
                {
                    UE_LOG(LogTemp, Warning, TEXT("Impactwd123 Point: %s"), *ImpactPoint.ToString());
                    Weapon->SpawnStoneSkill(ImpactPoint); 
                }
            }
        }
    } 
}