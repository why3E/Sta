// Fill out your copyright notice in the Description page of Project Settings.

#include "MyStoneWeapon.h"
#include "MyStoneWave.h"
#include "MyStoneSkill.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

AMyStoneWeapon::AMyStoneWeapon()
{
    static ConstructorHelpers::FClassFinder<AActor> StoneWaveRef(TEXT("/Game/Weapon/MyStoneWave.MyStoneWave_C"));
    if (StoneWaveRef.Succeeded())
    {
        StoneWaveClass = StoneWaveRef.Class;
    }

    static ConstructorHelpers::FClassFinder<AActor> StoneSkillRef(TEXT("/Game/Weapon/MyStoneSkill.MyStoneSkill_C"));
    if (StoneSkillRef.Succeeded())
    {
        StoneSkillClass = StoneSkillRef.Class;
    }

    StoneSkillSocket = TEXT("StonePosition");
    WeaponType = EWeaponType::WT_Stone;
}

void AMyStoneWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AMyStoneWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMyStoneWeapon::SpawnStoneWave(FVector FireLocation)
{
    if (StoneWaveClass)
    {
        FVector SpawnLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = GetInstigator();

        AMyStoneWave* StoneWave = GetWorld()->SpawnActor<AMyStoneWave>(StoneWaveClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

        if (StoneWave)
        {
            unsigned short skill_id = Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id();

            StoneWave->SetID(skill_id);
            StoneWave->SetOwner(OwnerCharacter);

            g_c_skills.emplace(skill_id, StoneWave);

            if (g_c_skill_collisions.count(skill_id)) {
                while (!g_c_skill_collisions[skill_id].empty()) {
                    char skill_type = g_c_skill_collisions[skill_id].front();
                    g_c_skill_collisions[skill_id].pop();

                    g_c_skills[skill_id]->Overlap(skill_type);
                }
            }

            if (g_c_object_collisions.count(skill_id)) {
                while (!g_c_object_collisions[skill_id].empty()) {
                    unsigned short object_id = g_c_object_collisions[skill_id].front();
                    g_c_object_collisions[skill_id].pop();

                    g_c_skills[skill_id]->Overlap(object_id);
                }
            }

            StoneWave->Fire(FireLocation);
        }
    }
}

void AMyStoneWeapon::SpawnStoneSkill(FVector ImpactPoint)
{
    if (!StoneSkillClass || !OwnerCharacter)
    {
        UE_LOG(LogTemp, Error, TEXT("StoneSkillClass or OwnerCharacter is null!"));
        return;
    }

    USkeletalMeshComponent* MeshComp = OwnerCharacter->GetMesh();
    if (!MeshComp || !MeshComp->DoesSocketExist(StoneSkillSocket))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid mesh or socket not found: %s"), *StoneSkillSocket.ToString());
        return;
    }

    // 오너캐릭터 위치에서 ImpactPoint 방향으로 500만큼 떨어진 위치 계산
    FVector OwnerLocation = OwnerCharacter->GetActorLocation();
    FVector Direction = (ImpactPoint - OwnerLocation).GetSafeNormal();
    float OffsetDistance = 500.f;
    FVector SpawnLocation = OwnerLocation + Direction * OffsetDistance;

    TempStoneSkill = Cast<AMyStoneSkill>(GetWorld()->SpawnActor<AMyStoneSkill>(StoneSkillClass, SpawnLocation, FRotator::ZeroRotator));
    if (!TempStoneSkill)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn StoneSkill"));
        return;
    }

    unsigned short skill_id = Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id();

    TempStoneSkill->SetID(skill_id);
    TempStoneSkill->SetOwner(OwnerCharacter);

    g_c_skills.emplace(skill_id, TempStoneSkill);

    if (g_c_skill_collisions.count(skill_id)) {
        while (!g_c_skill_collisions[skill_id].empty()) {
            char skill_type = g_c_skill_collisions[skill_id].front();
            g_c_skill_collisions[skill_id].pop();

            g_c_skills[skill_id]->Overlap(skill_type);
        }
    }

    if (g_c_object_collisions.count(skill_id)) {
        while (!g_c_object_collisions[skill_id].empty()) {
            unsigned short object_id = g_c_object_collisions[skill_id].front();
            g_c_object_collisions[skill_id].pop();

            g_c_skills[skill_id]->Overlap(object_id);
        }
    }

    StoneSkillImpactPoint = ImpactPoint;

    UE_LOG(LogTemp, Warning, TEXT("StoneSkill spawned at: %s"), *SpawnLocation.ToString());
}

void AMyStoneWeapon::ShootStoneSkill()
{
    if (!TempStoneSkill)
    {
        UE_LOG(LogTemp, Warning, TEXT("TempStoneSkill is null, nothing to shoot"));
        return;
    }

    // 실제 발사
    TempStoneSkill->Fire(StoneSkillImpactPoint);
    TempStoneSkill = nullptr;
}