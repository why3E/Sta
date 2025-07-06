#include "MyAltarMain.h"
#include "MyAltarTorch.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AMyAltarMain::AMyAltarMain()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    AltarMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh1"));
    AltarMesh1->SetupAttachment(SceneRoot);

    AltarMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh2"));
    AltarMesh2->SetupAttachment(SceneRoot);

    AltarMesh3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh3"));
    AltarMesh3->SetupAttachment(SceneRoot);

    AltarMesh4 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AltarMesh4"));
    AltarMesh4->SetupAttachment(SceneRoot);
}

// 내부 상태 변수
TArray<UStaticMeshComponent*> AltarMeshes;
TArray<float> StartZOffsets;
TArray<float> TargetZOffsets;
TArray<bool> bIsInterpolating;

void AMyAltarMain::BeginPlay()
{
    Super::BeginPlay();

    // 제단 초기화
    AltarMeshes = { AltarMesh1, AltarMesh2, AltarMesh3, AltarMesh4 };
    const int32 NumAltars = AltarMeshes.Num();

    StartZOffsets.Init(-750.0f, NumAltars);
    TargetZOffsets.Init(-50.0f, NumAltars);
    bIsInterpolating.Init(false, NumAltars);

    for (int32 i = 0; i < NumAltars; ++i)
    {
        if (AltarMeshes[i])
        {
            AltarMeshes[i]->SetVisibility(false);
            FVector Loc = AltarMeshes[i]->GetRelativeLocation();
            Loc.Z = StartZOffsets[i];
            AltarMeshes[i]->SetRelativeLocation(Loc);
        }
    }
	
    // Torch 스폰 및 오너 설정
    for (AActor* Target : TorchesSpawnTargets)
    {
        if (!Target || !TorchClass) continue;

        FVector SpawnLocation = Target->GetActorLocation();
        FRotator SpawnRotation = Target->GetActorRotation();

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        AMyAltarTorch* SpawnedTorch = GetWorld()->SpawnActor<AMyAltarTorch>(
            TorchClass, SpawnLocation, SpawnRotation, SpawnParams);

        if (SpawnedTorch)
        {   // 필요 시 MainAltar 설정
            SpawnedTorch->SetMainAltar(this);
        }
    }
}

void AMyAltarMain::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    for (int32 i = 0; i < AltarMeshes.Num(); ++i)
    {
        if (bIsInterpolating[i] && AltarMeshes[i])
        {
            FVector CurrentLoc = AltarMeshes[i]->GetRelativeLocation();
            float NewZ = FMath::FInterpTo(CurrentLoc.Z, TargetZOffsets[i], DeltaTime, 2.0f);
            CurrentLoc.Z = NewZ;
            AltarMeshes[i]->SetRelativeLocation(CurrentLoc);

            if (FMath::IsNearlyEqual(NewZ, TargetZOffsets[i], 1.0f))
            {
                bIsInterpolating[i] = false;
                CurrentLoc.Z = TargetZOffsets[i];
                AltarMeshes[i]->SetRelativeLocation(CurrentLoc);
            }
        }
    }
}

void AMyAltarMain::NotifyTorchActivated()
{
    ActivatedTorchCount++;
    UpdateAltarState();
}

void AMyAltarMain::UpdateAltarState()
{
    UE_LOG(LogTemp, Warning, TEXT("Activated Torch Count: %d"), ActivatedTorchCount);

    if (ActivatedTorchCount <= AltarMeshes.Num())
    {
        int32 Index = ActivatedTorchCount - 1;
        if (AltarMeshes[Index])
        {
            AltarMeshes[Index]->SetVisibility(true);
            bIsInterpolating[Index] = true;
        }
    }

    if (ActivatedTorchCount >= 4 && ChestClass)
    {
		UE_LOG(LogTemp, Warning, TEXT("All torches activated, spawning chest."));
        FVector SpawnLoc = GetActorLocation();
		SpawnLoc.Z += 10.f;
        GetWorld()->SpawnActor<AActor>(ChestClass, SpawnLoc , FRotator::ZeroRotator);
    }
}
