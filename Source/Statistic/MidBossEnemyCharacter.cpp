#include "MidBossEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"
#include "MyStoneWave.h"
#include "MyStoneSkill.h"

#include "DrawDebugHelpers.h" // 꼭 추가!
#include "MidBossEnemyCharacter.h"
#include "ProceduralMeshComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"

#include "UObject/ConstructorHelpers.h"
#include "NiagaraFunctionLibrary.h"

AMidBossEnemyCharacter::AMidBossEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetHiddenInGame(false);

	GetMesh()->SetupAttachment(GetCapsuleComponent());
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetHiddenInGame(false);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BossMesh(TEXT("/Game/Wood_Monster/CharacterParts/Meshes/SK_wood_giant_01_a.SK_wood_giant_01_a"));
	if (BossMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(BossMesh.Object);
	}

	GetCharacterMovement()->GravityScale = 1.0f;
	GetCharacterMovement()->BrakingDecelerationFalling = 2048.f;
	GetCharacterMovement()->JumpZVelocity = 0.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 200.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1500.f;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	bUseControllerRotationYaw = false;

    ProcMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMeshComponent"));
    ProcMeshComponent->SetupAttachment(RootComponent);
    ProcMeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));

	auto CreateCollision = [&](FName Name, FName SocketName) -> UCapsuleComponent*
	{
		UCapsuleComponent* Capsule = CreateDefaultSubobject<UCapsuleComponent>(Name);
		Capsule->SetupAttachment(GetMesh(), SocketName);
		Capsule->SetMobility(EComponentMobility::Movable);
		Capsule->InitCapsuleSize(15.f, 30.f);
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Capsule->SetCollisionProfileName(TEXT("BlockAll"));
		Capsule->SetGenerateOverlapEvents(true);
		return Capsule;
	};

	HeadCollision = CreateCollision(TEXT("HeadCollision"), TEXT("HeadSocket"));
	ChestCollision = CreateCollision(TEXT("ChestCollision"), TEXT("ChestSocket"));
	HipCollision = CreateCollision(TEXT("HipCollision"), TEXT("HipSocket"));
	LeftArmUpperCollision  = CreateCollision(TEXT("LeftArmUpperCollision"), TEXT("LeftUpperArmSocket"));
	LeftArmLowerCollision  = CreateCollision(TEXT("LeftArmLowerCollision"), TEXT("LeftLowerArmSocket"));
	RightArmUpperCollision = CreateCollision(TEXT("RightArmUpperCollision"), TEXT("RightUpperArmSocket"));
	RightArmLowerCollision = CreateCollision(TEXT("RightArmLowerCollision"), TEXT("RightLowerArmSocket"));
	LeftLegUpperCollision  = CreateCollision(TEXT("LeftLegUpperCollision"), TEXT("LeftUpperLegSocket"));
	LeftLegLowerCollision  = CreateCollision(TEXT("LeftLegLowerCollision"), TEXT("LeftLowerLegSocket"));
	RightLegUpperCollision = CreateCollision(TEXT("RightLegUpperCollision"), TEXT("RightUpperLegSocket"));
	RightLegLowerCollision = CreateCollision(TEXT("RightLegLowerCollision"), TEXT("RightLowerLegSocket"));

	BaseAttackSocketName = TEXT("Attack_Socket");
	LaserAttackSocketName = TEXT("WindLaserSocket");

	static ConstructorHelpers::FClassFinder<AActor> StoneWaveRef(TEXT("/Game/Weapon/MyStoneWave.MyStoneWave_C"));
	if (StoneWaveRef.Succeeded()) StoneWaveClass = StoneWaveRef.Class;
	static ConstructorHelpers::FClassFinder<AActor> StoneSkillRef(TEXT("/Game/Weapon/MyStoneSkill.MyStoneSkill_C"));
	if (StoneSkillRef.Succeeded()) StoneSkillClass = StoneSkillRef.Class;
	static ConstructorHelpers::FClassFinder<AActor> WindCutterRef(TEXT("/Game/Weapon/MyWindCutter.MyWindCutter_C"));
	if (WindCutterRef.Succeeded()) WindCutterClass = WindCutterRef.Class;
	static ConstructorHelpers::FClassFinder<AActor> WindSkillRef(TEXT("/Game/Weapon/MyWindSkill.MyWindSkill_C"));
	if (WindSkillRef.Succeeded()) WindSkillClass = WindSkillRef.Class;
	static ConstructorHelpers::FClassFinder<AActor> WindLaserRef(TEXT("/Game/Weapon/MyWindLaser.MyWindLaser_C"));
	if (WindLaserRef.Succeeded()) WindLaserClass = WindLaserRef.Class;
}

void AMidBossEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &AMidBossEnemyCharacter::SkillAttack, 3.0f, false);

	MontageToHitCapsuleMap.Add(TEXT("WindLaser"), RightArmLowerCollision);
	MontageToHitCapsuleMap.Add(TEXT("WindCutter"), LeftArmLowerCollision);
	MontageToHitCapsuleMap.Add(TEXT("StoneWave"), ChestCollision);
	MontageToHitCapsuleMap.Add(TEXT("StoneThrow"), HipCollision);
	MontageToHitCapsuleMap.Add(TEXT("WindTonado"), HipCollision);

	GetWorldTimerManager().SetTimer(
        AttackTimerHandle, // 헤더에 FTimerHandle AttackTimerHandle; 선언 필요
        this,
        &AMidBossEnemyCharacter::Die,
        3.0f,
        false
    );
}

void AMidBossEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CachedPlayerCharacter)
	{
		FVector Direction = (CachedPlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FRotator NewRot = FMath::RInterpTo(GetActorRotation(), Direction.Rotation(), DeltaTime, 3.0f);
		SetActorRotation(NewRot);
	}
}

void AMidBossEnemyCharacter::Attack()
{
	if (AttackMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			TArray<FName> Sections = { TEXT("WindLaser"), TEXT("WindCutter"), TEXT("StoneWave") };
			int32 RandomIndex = FMath::RandRange(0, Sections.Num() - 1);
			FName SelectedSection = Sections[RandomIndex];

			float PlayRate = (SelectedSection == TEXT("WindLaser")) ? 0.5f : 1.0f;

			bIsPlayingMontageSection = true; // 몽타주 시작
			AnimInstance->Montage_Play(AttackMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(SelectedSection, AttackMontage);
			SpawnWeakPointEffectForCurrentSection(SelectedSection);
		}
	}
}

void AMidBossEnemyCharacter::SkillAttack()
{
	if (AttackMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			TArray<FName> Sections = { TEXT("StoneThrow"), TEXT("WindTonado") };
			int32 RandomIndex = FMath::RandRange(0, Sections.Num() - 1);
			FName SelectedSection = Sections[RandomIndex];

			bIsPlayingMontageSection = true; // 몽타주 시작
			AnimInstance->Montage_Play(AttackMontage, 0.3f);
			AnimInstance->Montage_JumpToSection(SelectedSection, AttackMontage);
			SpawnWeakPointEffectForCurrentSection(SelectedSection);
		}
	}
}

void AMidBossEnemyCharacter::PlayHitAttackMontage()
{
	if (HitAttackMontage)
	{
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(HitAttackMontage, 1.0f);
			RemoveWeakPointEffect();
		}
	}
}

void AMidBossEnemyCharacter::SpawnWeakPointEffectForCurrentSection(FName SectionName)
{
	if (!WeakPointEffect || SectionName.IsNone()) return;

	if (UCapsuleComponent* HitCapsule = MontageToHitCapsuleMap.FindRef(SectionName))
	{
		FName SocketName = HitCapsule->GetAttachSocketName();
		if (SocketName != NAME_None && GetMesh())
		{
			ActiveWeakPointEffect = UNiagaraFunctionLibrary::SpawnSystemAttached(
				WeakPointEffect, GetMesh(), SocketName,
				FVector::ZeroVector, FRotator::ZeroRotator,
				EAttachLocation::SnapToTargetIncludingScale, true);
		}
	}
}

void AMidBossEnemyCharacter::RemoveWeakPointEffect()
{
	if (ActiveWeakPointEffect)
	{
		ActiveWeakPointEffect->DestroyComponent();
		ActiveWeakPointEffect = nullptr;
	}
}

void AMidBossEnemyCharacter::OnHitCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance || !AnimInstance->Montage_IsPlaying(AttackMontage)) return;

	FName CurrentSection = AnimInstance->Montage_GetCurrentSection(AttackMontage);
	if (CurrentSection.IsNone()) return;

	if (MontageToHitCapsuleMap.Contains(CurrentSection) && MontageToHitCapsuleMap[CurrentSection] == OverlappedComp)
	{
		AnimInstance->Montage_Stop(0.1f, AttackMontage);
		PlayHitAttackMontage();
		bIsPlayingMontageSection = false; // 몽타주 종료
	}
}

FVector AMidBossEnemyCharacter::GetCurrentImpactPoint() { FindPlayerCharacter(); return CachedPlayerCharacter ? CachedPlayerCharacter->GetActorLocation() : GetActorLocation(); }
FRotator AMidBossEnemyCharacter::GetCurrentImpactRot() { FindPlayerCharacter(); return CachedPlayerCharacter ? (CachedPlayerCharacter->GetActorLocation() - GetActorLocation()).GetSafeNormal().Rotation() : GetActorRotation(); }
FVector AMidBossEnemyCharacter::GetFireLocation() { FindPlayerCharacter(); return CachedPlayerCharacter ? CachedPlayerCharacter->GetActorLocation() : GetActorLocation() + GetActorForwardVector() * 500.f; }

void AMidBossEnemyCharacter::FindPlayerCharacter()
{
	TArray<AActor*> PlayerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), PlayerActors);
	CachedPlayerCharacter = PlayerActors.Num() > 0 ? Cast<APlayerCharacter>(PlayerActors[FMath::RandRange(0, PlayerActors.Num() - 1)]) : nullptr;
}

TArray<FVector> AMidBossEnemyCharacter::GenerateWindTonadoLocations(int32 Count, float MinRadius, float MaxRadius, float MinDistance)
{
    TArray<FVector> Result;
    FVector Origin = GetActorLocation();

    for (int32 i = 0; i < Count; ++i)
    {
        float Angle = FMath::FRandRange(0.f, 360.f);
        float Radius = FMath::FRandRange(MinRadius, MaxRadius);
        FVector Offset = FVector(FMath::Cos(FMath::DegreesToRadians(Angle)), FMath::Sin(FMath::DegreesToRadians(Angle)), 0.f) * Radius;

        bool bOverlap = false;
        for (const FVector& Existing : Result)
        {
            if (FVector::Dist(Origin + Offset, Existing) < MinDistance)
            {
                bOverlap = true;
                break;
            }
        }

        if (bOverlap)
        {
            --i;
            continue;
        }

        Result.Add(Origin + Offset);
    }

    return Result;
}


void AMidBossEnemyCharacter::Die()
{
    TargetBoneName = GetBoneName();

    // (1) 복사 및 메시 생성
    CopySkeletalMeshToProcedural(0);

    FTransform MeshTransform = GetMesh()->GetComponentTransform();
    ProcMeshComponent->SetWorldTransform(MeshTransform);

    ProcMeshComponent->SetWorldTransform(GetMesh()->GetComponentTransform());
    ProcMeshComponent->SetVisibility(true, true);
    ProcMeshComponent->SetHiddenInGame(false, true);

    // 디버그: 위치/스케일/회전 로그
    UE_LOG(LogTemp, Warning, TEXT("[Debug] Mesh Location: %s, ProcMesh Location: %s"), 
        *GetMesh()->GetComponentLocation().ToString(), 
        *ProcMeshComponent->GetComponentLocation().ToString());
    UE_LOG(LogTemp, Warning, TEXT("[Debug] Mesh Scale: %s, ProcMesh Scale: %s"), 
        *GetMesh()->GetComponentScale().ToString(), 
        *ProcMeshComponent->GetComponentScale().ToString());
    UE_LOG(LogTemp, Warning, TEXT("[Debug] Mesh Rot: %s, ProcMesh Rot: %s"), 
        *GetMesh()->GetComponentRotation().ToString(), 
        *ProcMeshComponent->GetComponentRotation().ToString());

    // 머티리얼: 반드시 마지막에 실제 메시용으로!
    for (int32 i = 0; i < GetMesh()->GetNumMaterials(); i++)
    {
        UMaterialInterface* Mat = GetMesh()->GetMaterial(i);
        if (Mat) ProcMeshComponent->SetMaterial(i, Mat);
    }

    // 버텍스 파란 점 찍기
    for (int32 i = 0; i < FMath::Min(FilteredVerticesArray.Num(), 100); i++)
    {
        FVector WorldPos = ProcMeshComponent->GetComponentTransform().TransformPosition(FilteredVerticesArray[i]);
        DrawDebugPoint(GetWorld(), WorldPos, 20.f, FColor::Blue, false, 10.f);
    }

    // (3) SkeletalMesh 숨기기
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCanEverAffectNavigation(false);
    GetMesh()->SetVisibility(false, true);
    GetMesh()->SetHiddenInGame(true, true);


}

FName AMidBossEnemyCharacter::GetBoneName() const
{
    return TEXT("spine_03");
}

void AMidBossEnemyCharacter::CopySkeletalMeshToProcedural(int32 LODIndex)
{
    if (!GetMesh() || !ProcMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("CopySkeletalMeshToProcedural: SkeletalMeshComponent or ProcMeshComp is null."));
        return;
    }

    // Skeletal Mesh → ProcMesh 위치/회전 맞추기 (스케일은 별도 조정)
    ProcMeshComponent->SetWorldLocation(GetMesh()->GetComponentLocation());
    ProcMeshComponent->SetWorldRotation(GetMesh()->GetComponentRotation());

    USkeletalMesh* SkeletalMesh = GetMesh()->GetSkeletalMeshAsset();
    if (!SkeletalMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("CopySkeletalMeshToProcedural: SkeletalMesh is null."));
        return;
    }

    const FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();
    if (!RenderData || !RenderData->LODRenderData.IsValidIndex(LODIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("CopySkeletalMeshToProcedural: LODRenderData[%d] is not valid."), LODIndex);
        return;
    }

    const FSkeletalMeshLODRenderData& LODRenderData = RenderData->LODRenderData[LODIndex];

    // "Local Space" 버텍스를 그대로 사용 (ProcMeshComponent 기준)
    FVector TargetBoneLocation = GetMesh()->GetBoneLocation(TargetBoneName);

    UE_LOG(LogTemp, Warning, TEXT("TargetBoneName: %s, TargetBoneLocation: %s, Distance: %.2f"),
        *TargetBoneName.ToString(), *TargetBoneLocation.ToString(), CreateProceduralMeshDistance);

    VertexIndexMap.Reset();
    FilteredVerticesArray.Reset();
    Normals.Reset();
    Tangents.Reset();
    UV.Reset();
    Colors.Reset();
    Indices.Reset();

    int32 vertexCounter = 0;
    int32 totalVertices = 0;
    int32 filteredVertices = 0;

    FTransform MeshTransform = GetMesh()->GetComponentTransform();

    for (const FSkelMeshRenderSection& Section : LODRenderData.RenderSections)
    {
        const int32 NumSourceVertices = Section.NumVertices;
        const int32 BaseVertexIndex = Section.BaseVertexIndex;

        for (int32 i = 0; i < NumSourceVertices; i++)
        {
            totalVertices++;
            const int32 VertexIndex = i + BaseVertexIndex;
            const FVector3f SkinnedVectorPos = LODRenderData.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);

            // "World" 좌표는 필터링(거리 비교)에만 사용, 실제 저장은 "Local"로!
            FVector WorldVertexPosition = MeshTransform.TransformPosition(FVector(SkinnedVectorPos));
            float DistanceToBone = FVector::Dist(WorldVertexPosition, TargetBoneLocation);

            if (DistanceToBone <= CreateProceduralMeshDistance)
            {
                filteredVertices++;
                // **꼭 Local Space!**
                FilteredVerticesArray.Add(FVector(SkinnedVectorPos));
                VertexIndexMap.Add(VertexIndex, vertexCounter);
                vertexCounter++;

                const FVector3f Normal = LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex);
                const FVector3f TangentX = LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex);
                const FVector2f SourceUVs = LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, 0);
                Normals.Add(FVector(Normal));
                Tangents.Add(FProcMeshTangent(FVector(TangentX), false));
                UV.Add(FVector2D(SourceUVs));
                Colors.Add(FColor(0, 0, 0, 255));
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Vertices in original mesh: %d"), totalVertices);
    UE_LOG(LogTemp, Warning, TEXT("Vertices within %.1f units of bone '%s': %d"),
        CreateProceduralMeshDistance, *TargetBoneName.ToString(), filteredVertices);

    const FRawStaticIndexBuffer16or32Interface* IndexBuffer = LODRenderData.MultiSizeIndexContainer.GetIndexBuffer();
    if (!IndexBuffer)
    {
        UE_LOG(LogTemp, Warning, TEXT("CopySkeletalMeshToProcedural: Index buffer is null."));
        return;
    }

    const int32 NumIndices = IndexBuffer->Num();
    int32 SkippedTriangleCount = 0;
    int32 TotalTriangles = NumIndices / 3;
    int32 CreatedTriangles = 0;

    for (int32 i = 0; i < NumIndices; i += 3)
    {
        int32 OldIndex0 = static_cast<int32>(IndexBuffer->Get(i));
        int32 OldIndex1 = static_cast<int32>(IndexBuffer->Get(i + 1));
        int32 OldIndex2 = static_cast<int32>(IndexBuffer->Get(i + 2));

        int32 NewIndex0 = VertexIndexMap.Contains(OldIndex0) ? VertexIndexMap[OldIndex0] : -1;
        int32 NewIndex1 = VertexIndexMap.Contains(OldIndex1) ? VertexIndexMap[OldIndex1] : -1;
        int32 NewIndex2 = VertexIndexMap.Contains(OldIndex2) ? VertexIndexMap[OldIndex2] : -1;

        // **삼각형 winding order 뒤집기도 실험 (아래 라인 두 줄 중 하나만 주석 해제)**
        // winding order: 정방향
        // if (조건) { Indices.Add(NewIndex0); Indices.Add(NewIndex1); Indices.Add(NewIndex2); CreatedTriangles++; }
        // winding order: 역방향 (삼각형 면이 뒤집혀 있다면 이걸로 실험)
        if (NewIndex0 < 0 || NewIndex1 < 0 || NewIndex2 < 0 ||
            NewIndex0 == NewIndex1 || NewIndex1 == NewIndex2 || NewIndex2 == NewIndex0 ||
            NewIndex0 >= FilteredVerticesArray.Num() || NewIndex1 >= FilteredVerticesArray.Num() || NewIndex2 >= FilteredVerticesArray.Num())
        {
            SkippedTriangleCount++;
            continue;
        }
        else
        {
            // 아래 중 하나만 주석 해제해서 테스트!
            Indices.Add(NewIndex0); Indices.Add(NewIndex1); Indices.Add(NewIndex2); CreatedTriangles++;
            // Indices.Add(NewIndex2); Indices.Add(NewIndex1); Indices.Add(NewIndex0); CreatedTriangles++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Triangles in original mesh: %d"), TotalTriangles);
    UE_LOG(LogTemp, Warning, TEXT("Created triangles: %d"), CreatedTriangles);
    UE_LOG(LogTemp, Warning, TEXT("Skipped degenerate or invalid triangles: %d"), SkippedTriangleCount);

    ProcMeshComponent->CreateMeshSection(0, FilteredVerticesArray, Indices, Normals, UV, Colors, Tangents, true);
}