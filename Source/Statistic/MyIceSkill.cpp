// Fill out your copyright notice in the Description page of Project Settings.
#include "MyIceSkill.h"
#include "NiagaraFunctionLibrary.h"



AMyIceSkill::AMyIceSkill()
{
    SetElement(EClassType::CT_Ice);

    PrimaryActorTick.bCanEverTick = true;

    // 메시 기반 콜리전 설정
    CollisionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionMesh"));
    RootComponent = CollisionMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/sA_IceSkillPack2_Remaster/Meshes/SM_Ice_Wall_1.SM_Ice_Wall_1"));
    if (MeshAsset.Succeeded())
    {
        CollisionMesh->SetStaticMesh(MeshAsset.Object);
        CollisionMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        CollisionMesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
        CollisionMesh->SetGenerateOverlapEvents(true);
        CollisionMesh->SetVisibility(false); // 보이지 않게
        CollisionMesh->SetHiddenInGame(true); // 게임 중 숨김
    }

    // 나이아가라 컴포넌트 설정
    IceWallNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("IceWallNiagaraComponent"));
    IceWallNiagaraComponent->SetupAttachment(CollisionMesh);
    IceWallNiagaraComponent->SetVisibility(true);
    IceWallNiagaraComponent->OnSystemFinished.AddDynamic(this, &AMyIceSkill::OnIceWallEffectFinished);
}

void AMyIceSkill::BeginPlay()
{
    Super::BeginPlay();

    // Tick 기반 충돌 체크 시작
    //GetWorld()->GetTimerManager().SetTimer(CheckOverlapTimerHandle, this, &AMyIceSkill::CheckOverlappingActors, 1.0f, true);
   
}

void AMyIceSkill::PostInitializeComponents()
{
    Super::PostInitializeComponents();

}

void AMyIceSkill::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AMyIceSkill::SpawnIceSkill(FVector Location, FRotator Rotation)
{
    SetActorLocation(Location);
    SetActorRotation(Rotation);

    if (IceWallEffect)
    {
        IceWallNiagaraComponent->SetAsset(IceWallEffect);
        IceWallNiagaraComponent->Activate(true); // 이펙트 실행
    }

    
    
}

// 효과 종료 시 호출될 함수 정의
void AMyIceSkill::OnIceWallEffectFinished(UNiagaraComponent* PSystem)
{
    Destroy();
}

void AMyIceSkill::BreakAndDestroy()
{
    if (IceWallBreakEffect) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), IceWallBreakEffect, GetActorLocation());
    }

    Destroy();
}

void AMyIceSkill::SmallAndDestroy()
{
    SmallCallCount++;

    // 1~4번째: 크기만 줄임
    if (SmallCallCount < 5)
    {
        float ScaleFactor = 0.9f;
        SetActorScale3D(GetActorScale3D() * ScaleFactor);
        return;
    }

    // 5번째: 충돌 비활성화 후 점점 작아지게(보간)
    if (SmallCallCount == 5)
    {
        // 충돌 비활성화
        if (CollisionMesh)
        {
            CollisionMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        }

        // 타이머로 점점 작아지게
        TargetScale = 0.1f; // 최종 스케일
        ShrinkInterpSpeed = 2.0f; // 보간 속도
        GetWorld()->GetTimerManager().SetTimer(
            ShrinkTimerHandle,
            this,
            &AMyIceSkill::ShrinkTick,
            0.02f,
            true
        );
    }
}

void AMyIceSkill::ShrinkTick()
{
    FVector CurrentScale = GetActorScale3D();
    FVector TargetVec(TargetScale, TargetScale, TargetScale);

    FVector NewScale = FMath::VInterpTo(CurrentScale, TargetVec, GetWorld()->GetDeltaSeconds(), ShrinkInterpSpeed);
    SetActorScale3D(NewScale);

    if (NewScale.GetMin() <= TargetScale + 0.01f)
    {
        GetWorld()->GetTimerManager().ClearTimer(ShrinkTimerHandle);
        Destroy();
    }
}