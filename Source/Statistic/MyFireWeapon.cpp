// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFireWeapon.h"
#include "MyFireBall.h"
#include "MyFireSkill.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/PoseableMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "MyPlayerVisualInterface.h"

AMyFireWeapon::AMyFireWeapon()
{
	static ConstructorHelpers::FClassFinder<AActor>FireBallRef(TEXT("/Game/Weapon/MyFireBall.MyFireBall_C"));
	if (FireBallRef.Succeeded())
	{
		FireBallClass = FireBallRef.Class;
	}
	static ConstructorHelpers::FClassFinder<AActor>FireSkillRef(TEXT("/Game/Weapon/MyFireSkill.MyFireSkill_C"));
	if (FireSkillRef.Succeeded())
	{
		FireSkillClass = FireSkillRef.Class;
	}
	WeaponType = EWeaponType::WT_Fire;
	FireBallSocket = TEXT("FirePosition");
}

void AMyFireWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AMyFireWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMyFireWeapon::SpawnFireBall(FVector ImpactPoint)
{
	TempFireBall = Cast<AMyFireBall>(GetWorld()->SpawnActor(FireBallClass));
    UE_LOG(LogTemp, Error, TEXT("FireBall %d Spawning"), Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id());

    if (TempFireBall)
	{
		if (OwnerCharacter)
        {
            unsigned short skill_id = Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id();

            TempFireBall->SetID(skill_id);
            TempFireBall->SetOwner(OwnerCharacter);
            TempFireBall->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FireBallSocket);
            TempFireBall->ActivateNiagara();
            UE_LOG(LogTemp, Warning, TEXT("FireBall %d spawniiiing"), skill_id);

            g_skills.emplace(skill_id, TempFireBall);
            if (g_collisions.count(skill_id)) {
                while (!g_collisions[skill_id].empty()) {
                    unsigned short other_id = g_collisions[skill_id].front();
                    g_collisions[skill_id].pop();

                    if (g_skills.count(other_id)) {
                        TempFireBall->Overlap(g_skills[other_id]);
                        g_skills[other_id]->Overlap(g_skills[skill_id]);
                        UE_LOG(LogTemp, Error, TEXT("Skill %d and %d Collision Succeed!"), skill_id, other_id);
                    }
                }
            }
            UE_LOG(LogTemp, Warning, TEXT("FireBall %d spawned"), skill_id);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerCharacter is null or GetOwner() is not a valid ACharacter!"));
        }
		FireLocation = ImpactPoint;
	}
}

void AMyFireWeapon::ShootFireBall()
{
	if (TempFireBall)
	{
		// 부모 액터로부터 부착 해제
		TempFireBall->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		TempFireBall->Fire(FireLocation);
		TempFireBall = nullptr;
	}
}

void AMyFireWeapon::SpawnFireSkill(FVector TargetLocation, FRotator TargetRotation)
{
    if (!FireSkillClass)
    {
        UE_LOG(LogTemp, Error, TEXT("FireSkillClass is not set!"));
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    int32 TotalObjects = 5; // 총 생성할 오브젝트 수
    float OffsetDistance = 200.f; // 오브젝트 간 거리
    FVector Forward = TargetRotation.Vector(); // 플레이어가 보는 방향
    FVector Right = FRotationMatrix(TargetRotation).GetUnitAxis(EAxis::Y); // 플레이어 기준 오른쪽 방향
    FVector Origin = TargetLocation; // 기준점: TargetLocation에서 앞쪽으로 이동

    int32 Half = TotalObjects / 2;

    for (int32 i = 0; i < TotalObjects; ++i)
    {
        int32 OffsetIndex = i - Half;
        FVector SpawnLocation = Origin + (Right * OffsetDistance * OffsetIndex);

        // 지형 높이 확인
        FHitResult HitResult;
        FVector Start = SpawnLocation + FVector(0.0f, 0.0f, 500.0f); // 위에서 아래로 라인트레이스
        FVector End = SpawnLocation - FVector(0.0f, 0.0f, 500.0f);   // 아래로 500 유닛

        if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
        {
            // 지형의 충돌 지점 높이로 Z값 조정
            SpawnLocation.Z = HitResult.ImpactPoint.Z;
        }

        FTransform SpawnTransform(TargetRotation, SpawnLocation);

        // FireSkill 생성
        AMyFireSkill* FireSkill = GetWorld()->SpawnActorDeferred<AMyFireSkill>(
            FireSkillClass,
            SpawnTransform,
            OwnerCharacter,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

        if (FireSkill)
        {
            unsigned short skill_id = Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id();

            FireSkill->SetID(i + skill_id);
            FireSkill->SetOwner(OwnerCharacter);
            FireSkill->SpawnFireWall(SpawnLocation, TargetRotation);

            g_skills.emplace(i + skill_id, FireSkill);
            UGameplayStatics::FinishSpawningActor(FireSkill, SpawnTransform);
            
            if (g_collisions.count(skill_id)) {
                while (!g_collisions[skill_id].empty()) {
                    unsigned short other_id = g_collisions[skill_id].front();
                    g_collisions[skill_id].pop();

                    if (g_skills.count(other_id)) {
                        FireSkill->Overlap(g_skills[other_id]);
                        g_skills[other_id]->Overlap(g_skills[skill_id]);
                        UE_LOG(LogTemp, Error, TEXT("Skill %d and %d Collision Succeed!"), skill_id, other_id);
                    }
                }
            }
            //UE_LOG(LogTemp, Warning, TEXT("FireSkill %d spawned at location: %s"), i + Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id(), *SpawnLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn FireSkill %d!"), i + 1);
        }
    }
}