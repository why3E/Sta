// Fill out your copyright notice in the Description page of Project Settings.


#include "MMWindSkillSpawn.h"
#include "AnimationWeaponInterface.h"
#include "ImpactPointInterface.h"
#include "MyWindWeapon.h"


void UMMWindSkillSpawn::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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
            FRotator ImpactRot = ImpactPointOwner->GetCurrentImpactRot(); // 회전값 가져오기
            UE_LOG(LogTemp, Warning, TEXT("Impact Point: %s"), *ImpactPoint.ToString());

            IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
            if (WeaponPawn)
            {
                AMyWindWeapon* Weapon = Cast<AMyWindWeapon>(WeaponPawn->GetWeapon());
                if (Weapon)
                {
                    Weapon->SpawnWindSkill(ImpactPoint); // 에너지볼 스폰 함수 호출 (위치 전달)
                }
            }
        }
    } 
}