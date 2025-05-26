// Fill out your copyright notice in the Description page of Project Settings.


#include "MyIceWeapon.h"
#include "MyIceArrow.h"
#include "MyIceSkill.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
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
    static ConstructorHelpers::FClassFinder<AActor> IceWallRef(TEXT("/Game/Weapon/MyIceSkill.MyIceSkill_C"));
    if (IceWallRef.Succeeded())
    {
        IceSkillClass =  IceWallRef.Class;
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

void AMyIceWeapon::SpawnIceSkill(FVector Location, FRotator Rotation)
{
    if (!IceSkillClass)
    {
        UE_LOG(LogTemp, Error, TEXT("IceWallClass is not set!"));
        return;
    }

    // 스폰 파라미터 설정
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    // 지형 높이 확인
    FHitResult HitResult;
    FVector Start = Location + FVector(0.0f, 0.0f, 500.0f); // 위에서 아래로 라인트레이스
    FVector End = Location - FVector(0.0f, 0.0f, 500.0f);   // 아래로 500 유닛

    FVector SpawnLocation = Location;
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
    {
        // 지형의 충돌 지점 높이로 Z값 조정
        SpawnLocation.Z = HitResult.ImpactPoint.Z;
    }

    FTransform SpawnTransform(Rotation, SpawnLocation);

    // 아이스월 생성
    AMyIceSkill* IceWall = GetWorld()->SpawnActorDeferred<AMyIceSkill>(
        IceSkillClass,
        SpawnTransform,
        OwnerCharacter,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (IceWall)
    {
        IceWall->SetOwner(OwnerCharacter);
        UGameplayStatics::FinishSpawningActor(IceWall, SpawnTransform);
        UE_LOG(LogTemp, Warning, TEXT("IceWall spawned at location: %s"), *SpawnLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn IceWall!"));
    }
}