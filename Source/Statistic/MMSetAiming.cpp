// Fill out your copyright notice in the Description page of Project Settings.


#include "MMSetAiming.h"
#include "AnimationWeaponInterface.h"
#include "MyIceWeapon.h"

void UMMSetAiming::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    if (MeshComp)
    {
        IAnimationWeaponInterface* WeaponPawn = Cast<IAnimationWeaponInterface>(MeshComp->GetOwner());
        if (WeaponPawn)
        {
            AMyIceWeapon* Weapon = Cast<AMyIceWeapon>(WeaponPawn->GetWeapon());
            if (Weapon)
            {
                UE_LOG(LogTemp, Warning, TEXT("Aiming Weapon"));
                Weapon->SetAiming(); // 에너지볼 스폰 함수 호출 (위치 전달)
            }
        }
        
    } 
}