// Fill out your copyright notice in the Description page of Project Settings.
#include "MyWindWeapon.h"
#include "MyWindSkill.h"
#include "MyWindCutter.h"
#include "GameFramework/Character.h"
#include "Components/PoseableMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "MyPlayerVisualInterface.h"

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
    BaseSocketName = TEXT("WindPosition");
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
            UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter Spawned"));
            TempWindCutter->SetOwner(OwnerCharacter);
            TempWindCutter->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, WindCutterSocket);
            TempWindCutter->ActivateNiagara();
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

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = GetInstigator();

    // 지형 높이 확인
    FHitResult HitResult;
    FVector Start = TargetLocation + FVector(0.0f, 0.0f, 500.0f); // 위에서 아래로 라인트레이스
    FVector End = TargetLocation - FVector(0.0f, 0.0f, 500.0f);   // 아래로 500 유닛

    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility))
    {
        // 지형의 충돌 지점 높이로 Z값 조정
        TargetLocation.Z = HitResult.ImpactPoint.Z;
    }

    // WindSkill 생성
    AMyWindSkill* WindSkill = GetWorld()->SpawnActor<AMyWindSkill>(WindSkillClass, TargetLocation, FRotator::ZeroRotator, SpawnParams);
    if (WindSkill)
    {
        WindSkill->SpawnWindTonado(TargetLocation);
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
		// 부모 액터로부터 부착 해제
		TempWindCutter->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		TempWindCutter->Fire(FireLocation);
		TempWindCutter = nullptr;
	}
}

