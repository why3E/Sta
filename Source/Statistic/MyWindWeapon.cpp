// Fill out your copyright notice in the Description page of Project Settings.
#include "MyWindWeapon.h"
#include "MyWindSkill.h"
#include "MyWindCutter.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "Components/PoseableMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "MyPlayerVisualInterface.h"

#include "SESSION.h"

AMyWindWeapon::AMyWindWeapon()
{
    static ConstructorHelpers::FClassFinder<AActor>WindCutterRef(TEXT("/Game/Weapon/MyWindCutter.MyWindCutter_C"));
    if (WindCutterRef.Succeeded())
    {
        WindCutterClass = WindCutterRef.Class;
    }
	static ConstructorHelpers::FClassFinder<AActor>WindSkillRef(TEXT("/Game/Weapon/MyWindSkill.MyWindSkill_C"));
    if (WindSkillRef.Succeeded())
    {
        WindSkillClass = WindSkillRef.Class;
    }
    WeaponType = EWeaponType::WT_Wind;
    WindCutterSocket = TEXT("WindPosition");
}

void AMyWindWeapon::BeginPlay()
{
	Super::BeginPlay();
}

void AMyWindWeapon::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMyWindWeapon::SpawnWindCutter(FVector ImpactPoint)
{
    TempWindCutter = Cast<AMyWindCutter>(GetWorld()->SpawnActor(WindCutterClass));

	if (TempWindCutter)
	{
		UE_LOG(LogTemp, Warning, TEXT("WindCutter Spawned"));
		if (OwnerCharacter)
        {
            
            unsigned short skill_id = Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id();

            TempWindCutter->SetID(skill_id);
            TempWindCutter->SetOwner(OwnerCharacter);
            TempWindCutter->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WindCutterSocket);
            TempWindCutter->ActivateNiagara();

            g_c_skills.emplace(skill_id, TempWindCutter);
            if (g_c_collisions.count(skill_id)) {
                while (!g_c_collisions[skill_id].empty()) {
                    unsigned short other_id = g_c_collisions[skill_id].front();
                    g_c_collisions[skill_id].pop();

                    if (g_c_skills.count(other_id)) {
                        TempWindCutter->Overlap(g_c_skills[other_id]);
                        g_c_skills[other_id]->Overlap(g_c_skills[skill_id]);
                        UE_LOG(LogTemp, Error, TEXT("Skill %d and %d Collision Succeed!"), skill_id, other_id);
                    }
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerCharacter is null or GetOwner() is not a valid ACharacter!"));
        }
		FireLocation = ImpactPoint;
	}
}

void AMyWindWeapon::SpawnWindSkill(FVector TargetLocation)
{
    if (!WindSkillClass)
    {
        UE_LOG(LogTemp, Error, TEXT("WindSkillClass is not set!"));
        return;
    }

    FTransform SpawnTransform(FRotator::ZeroRotator, TargetLocation);

    // WindSkill 생성
    AMyWindSkill* WindSkill = GetWorld()->SpawnActorDeferred<AMyWindSkill>(
        WindSkillClass,
        SpawnTransform,
        OwnerCharacter,
        nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );

    if (WindSkill)
    {
        unsigned short skill_id = Cast<APlayerCharacter>(OwnerCharacter)->get_skill_id();

        WindSkill->SetID(skill_id);
        WindSkill->SetOwner(OwnerCharacter);
		WindSkill->SpawnWindTonado(TargetLocation);

        g_c_skills.emplace(skill_id, WindSkill);
        UGameplayStatics::FinishSpawningActor(WindSkill, SpawnTransform);

        if (g_c_collisions.count(skill_id)) {
            while (!g_c_collisions[skill_id].empty()) {
                unsigned short other_id = g_c_collisions[skill_id].front();
                g_c_collisions[skill_id].pop();

                if (g_c_skills.count(other_id)) {
                    WindSkill->Overlap(g_c_skills[other_id]);
                    g_c_skills[other_id]->Overlap(g_c_skills[skill_id]);
                    UE_LOG(LogTemp, Error, TEXT("Skill %d and %d Collision Succeed!"), skill_id, other_id);
                }
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("WindSkill spawned at location: %s"), *TargetLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to spawn WindSkill!"));
    }
}

void AMyWindWeapon::ShootWindCutter()
{
    if (TempWindCutter)
    {
        if (WindCutterShootSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, WindCutterShootSound, GetActorLocation(), 5.0f);
        }
        // 부모 액터로부터 부착 해제
        TempWindCutter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        TempWindCutter->Fire(FireLocation);
        TempWindCutter = nullptr;
    }
}