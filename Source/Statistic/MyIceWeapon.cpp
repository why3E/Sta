// Fill out your copyright notice in the Description page of Project Settings.


#include "MyIceWeapon.h"
#include "MyIceArrow.h"
#include "PlayerCharacter.h"
#include "GameFramework/Character.h"

AMyIceWeapon::AMyIceWeapon()
{
    // WeaponMesh 생성 및 WeaponCollision(부모 루트)에 부착
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
    WeaponMesh->SetupAttachment(RootComponent); // WeaponCollision에 부착

    // 필요하다면 메시의 충돌 설정 등 추가
    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetVisibility(false);

    static ConstructorHelpers::FClassFinder<AActor> IceArrowRef(TEXT("/Game/Weapon/MyIceArrow.MyIceArrow_C"));
    if (IceArrowRef.Succeeded())
    {
        IceArrowClass =  IceArrowRef.Class;
    }

    WeaponType = EWeaponType::WT_Ice;

    IceSocket = TEXT("IcePosition");
}

void AMyIceWeapon::BeginPlay()
{
    Super::BeginPlay();

}
void AMyIceWeapon::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

}

void AMyIceWeapon::SetAiming()
{
    if (bIsAiming)
        return;
    bIsAiming = true;
    WeaponMesh->SetVisibility(true);
    TempIceArrow = Cast<AMyIceArrow>(GetWorld()->SpawnActor(IceArrowClass));
    UE_LOG(LogTemp, Error, TEXT("Ice Arrow %d Spawning"), Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id());

    if (TempIceArrow)
	{
		if (OwnerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("Ice Arrow Spawned"));
            // 소유자 설정
            unsigned short skill_id = Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id();

            TempIceArrow->SetID(skill_id);
            TempIceArrow->SetOwner(OwnerCharacter);

            // 소켓에 부착
            TempIceArrow->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, IceSocket);

            // 파티클 등 활성화
            TempIceArrow->ActivateNiagara();

            g_c_skills.emplace(skill_id, TempIceArrow);
            if (g_c_collisions.count(skill_id)) {
                while (!g_c_collisions[skill_id].empty()) {
                    unsigned short other_id = g_c_collisions[skill_id].front();
                    g_c_collisions[skill_id].pop();

                    if (g_c_skills.count(other_id)) {
                        TempIceArrow->Overlap(g_c_skills[other_id]);
                        g_c_skills[other_id]->Overlap(g_c_skills[skill_id]);
                        UE_LOG(LogTemp, Error, TEXT("Skill %d and %d Collision Succeed!"), skill_id, other_id);
                    }
                }
            }
        }
    }
}


void AMyIceWeapon::ShootIceArrow(FVector FirePoint)
{
    if (TempIceArrow)
	{
		// 부모 액터로부터 부착 해제
		TempIceArrow->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		TempIceArrow->Fire(FirePoint);
		TempIceArrow = nullptr;
	}
    bIsAiming = false;
    WeaponMesh->SetVisibility(false);
}

