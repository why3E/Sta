// Fill out your copyright notice in the Description page of Project Settings.


#include "MMIceWallSpawn.h"
#include "AnimationWeaponInterface.h"
#include "MyIceWeapon.h"
#include "ImpactPointInterface.h"

void UMMIceWallSpawn::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
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

            // 무기 인터페이스를 통해 FireBall 스폰
            IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
            if (WeaponPawn)
            {
                AMyIceWeapon* Weapon = Cast<AMyIceWeapon>(WeaponPawn->GetWeapon());
                if (Weapon)
                {
                    Weapon->SpawnIceSkill(ImpactPoint, ImpactRot); 
                }
            }
        }
    } 
}
