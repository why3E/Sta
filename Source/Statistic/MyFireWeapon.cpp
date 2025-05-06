// Fill out your copyright notice in the Description page of Project Settings.


#include "MyFireWeapon.h"
#include "MyFireBall.h"
#include "MyFireSkill.h"
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
	BaseSocketName = TEXT("FirePosition");
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

void AMyFireWeapon::SpawnFireBall()
{
	TempFireBall = Cast<AMyFireBall>(GetWorld()->SpawnActor(FireBallClass));

	if (TempFireBall)
	{
		UE_LOG(LogTemp, Warning, TEXT("FireBall1 Spawned"));
		if (OwnerCharacter)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter Spawned"));
            TempFireBall->SetOwner(OwnerCharacter);
            TempFireBall->AttachToComponent(OwnerCharacter->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, FireBallSocket);
            TempFireBall->ActivateNiagara();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("PlayerCharacter is null or GetOwner() is not a valid ACharacter!"));
        }

		// 발사할 위치를 선정합니다.
		SetFireLocation();
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
    float OffsetDistance = 100.f; // 오브젝트 간 거리
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

        // FireSkill 생성
        AMyFireSkill* FireSkill = GetWorld()->SpawnActor<AMyFireSkill>(FireSkillClass, SpawnLocation, TargetRotation, SpawnParams);
        if (FireSkill)
        {
            FireSkill->SpawnFireWall(SpawnLocation, TargetRotation);
            UE_LOG(LogTemp, Warning, TEXT("FireSkill %d spawned at location: %s"), i + 1, *SpawnLocation.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn FireSkill %d!"), i + 1);
        }
    }
}

void AMyFireWeapon::SetFireLocation()
{
	// 플레이어의 카메라에서 화면의 중앙으로 LineTrace를 진행합니다.
	IMyPlayerVisualInterface* PlayerCharacter = Cast<IMyPlayerVisualInterface>(GetOwner());

	UE_LOG(LogTemp, Warning, TEXT("PlayerCharacter: %s"), PlayerCharacter ? TEXT("Valid") : TEXT("Invalid"));
	if (PlayerCharacter)
	{
		// 충돌 결과 반환용
		FHitResult HitResult;
		// 시작 지점 (카메라의 위치)
		FVector Start = PlayerCharacter->GetPlayerCamera()->GetComponentLocation();
		// 종료 지점 (카메라 위치 + 카메라 전방벡터 * 20000)
		float Distance = 20000;
		FVector End = Start + (PlayerCharacter->GetPlayerCamera()->GetForwardVector() * Distance);
		// 파라미터 설정
		FCollisionQueryParams Params(SCENE_QUERY_STAT(Shoot), false, Owner);

		// 충돌 탐지
		bool bHasHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			Params
		);

		if (bHasHit)
		{
			FireLocation = HitResult.ImpactPoint;
		}
		else
		{
			FireLocation = End;
		}
		UE_LOG(LogTemp, Warning, TEXT("FireLocation: %s"), *FireLocation.ToString());
\
		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character)
		{
			FRotator PlayerRotator = Character->GetActorRotation();
			Character->SetActorRotation(FRotator(PlayerRotator.Pitch, Character->GetControlRotation().Yaw, PlayerRotator.Roll));
		}
	}
}