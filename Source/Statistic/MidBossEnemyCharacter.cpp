#include "MidBossEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"
#include "MyStoneWave.h"
#include "MyStoneSkill.h"


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
    ProcMeshComponent->SetVisibility(false);
    ProcMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
    // 🎯 반드시 먼저 설정!
    TargetBoneName = GetSecondBoneName();

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCanEverAffectNavigation(false);

    GetMesh()->SetVisibility(false);
    CopySkeletalMeshToProcedural(0);

    if (FilteredVerticesArray.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("❌ No vertices copied. Slicing aborted."));
        return;
    }

    if (!ProcMeshComponent->IsRegistered())
    {
        ProcMeshComponent->RegisterComponent();
    }
    ProcMeshComponent->bUseComplexAsSimpleCollision = false;
    ProcMeshComponent->SetVisibility(true);
    ProcMeshComponent->SetSimulatePhysics(true);

    FVector PlaneNormal = FVector(1.f, 0.f, 1.f).GetSafeNormal();
    SliceMeshAtBone(PlaneNormal, true);
}

FName AMidBossEnemyCharacter::GetSecondBoneName() const
{
    return TEXT("spine_04");
}

void AMidBossEnemyCharacter::CopySkeletalMeshToProcedural(int32 LODIndex)
{
    if (!GetMesh() || !ProcMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("CopySkeletalMeshToProcedural: SkeletalMeshComponent or ProcMeshComponent is null."));
        return;
    }

    FVector MeshLocation = GetMesh()->GetComponentLocation();
    FRotator MeshRotation = GetMesh()->GetComponentRotation();
    ProcMeshComponent->SetWorldLocation(MeshLocation);
    ProcMeshComponent->SetWorldRotation(MeshRotation);

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
    FTransform MeshTransform = GetMesh()->GetComponentTransform();
    FVector TargetBoneLocation = GetMesh()->GetBoneLocation(TargetBoneName);

    int32 VertexCounter = 0;
    for (const FSkelMeshRenderSection& Section : LODRenderData.RenderSections)
    {
        const int32 NumSourceVertices = Section.NumVertices;
        const int32 BaseVertexIndex = Section.BaseVertexIndex;

        for (int32 i = 0; i < NumSourceVertices; i++)
        {
            const int32 VertexIndex = i + BaseVertexIndex;
            const FVector3f SkinnedVectorPos = LODRenderData.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
            FVector WorldVertexPosition = MeshTransform.TransformPosition(FVector(SkinnedVectorPos));
            float DistanceToBone = FVector::Dist(WorldVertexPosition, TargetBoneLocation);

            if (DistanceToBone <= CreateProceduralMeshDistance)
            {
                VertexIndexMap.Add(VertexIndex, VertexCounter++);
                FilteredVerticesArray.Add(FVector(SkinnedVectorPos));
                Normals.Add(FVector(LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex)));
                Tangents.Add(FProcMeshTangent(FVector(LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex)), false));
                UV.Add(FVector2D(LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, 0)));
                Colors.Add(FColor(0, 0, 0, 255));
            }
        }
    }

    const FRawStaticIndexBuffer16or32Interface* IndexBuffer = LODRenderData.MultiSizeIndexContainer.GetIndexBuffer();
    if (!IndexBuffer)
    {
        UE_LOG(LogTemp, Warning, TEXT("CopySkeletalMeshToProcedural: Index buffer is null."));
        return;
    }

    const int32 NumIndices = IndexBuffer->Num();
    for (int32 i = 0; i < NumIndices; i += 3)
    {
        int32 OldIndex0 = IndexBuffer->Get(i);
        int32 OldIndex1 = IndexBuffer->Get(i + 1);
        int32 OldIndex2 = IndexBuffer->Get(i + 2);

        int32 NewIndex0 = VertexIndexMap.Contains(OldIndex0) ? VertexIndexMap[OldIndex0] : -1;
        int32 NewIndex1 = VertexIndexMap.Contains(OldIndex1) ? VertexIndexMap[OldIndex1] : -1;
        int32 NewIndex2 = VertexIndexMap.Contains(OldIndex2) ? VertexIndexMap[OldIndex2] : -1;

        if (NewIndex0 >= 0 && NewIndex1 >= 0 && NewIndex2 >= 0)
        {
            Indices.Add(NewIndex0);
            Indices.Add(NewIndex1);
            Indices.Add(NewIndex2);
        }
    }

    ProcMeshComponent->CreateMeshSection(0, FilteredVerticesArray, Indices, Normals, UV, Colors, Tangents, true);

    if (FilteredVerticesArray.Num() > 0)
    {
        ProcMeshComponent->ClearCollisionConvexMeshes();
        ProcMeshComponent->AddCollisionConvexMesh(FilteredVerticesArray);
    }

    ProcMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ProcMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    ProcMeshComponent->SetSimulatePhysics(false);
    ProcMeshComponent->SetEnableGravity(true);

    UMaterialInterface* SkeletalMeshMaterial = GetMesh()->GetMaterial(0);
    if (SkeletalMeshMaterial)
    {
        ProcMeshComponent->SetMaterial(0, SkeletalMeshMaterial);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SkeletalMesh has no material assigned."));
    }
}
void AMidBossEnemyCharacter::SliceMeshAtBone(FVector SliceNormal, bool bCreateOtherHalf)
{
    if (!GetMesh() || !ProcMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("SliceMeshAtBone: SkeletalMeshComponent or ProcMeshComponent is null."));
        return;
    }

    FVector BoneLocation = GetMesh()->GetBoneLocation(TargetBoneName);
    if (BoneLocation.IsNearlyZero())
    {
        UE_LOG(LogTemp, Error, TEXT("SliceMeshAtBone: Failed to get Bone '%s' location. Check if the bone exists."), *TargetBoneName.ToString());
        return;
    }

    UMaterialInterface* CapMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CutFace.M_CutFace"));
    if (!CapMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("SliceMeshAtBone: Failed to load Cap Material."));
    }

    UProceduralMeshComponent* OtherHalfMesh = nullptr;
    UKismetProceduralMeshLibrary::SliceProceduralMesh(
        ProcMeshComponent,
        BoneLocation,
        SliceNormal,
        bCreateOtherHalf,
        OtherHalfMesh,
        EProcMeshSliceCapOption::CreateNewSectionForCap,
        CapMaterial
    );

    if (!OtherHalfMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("SliceMeshAtBone: Failed to slice mesh at bone '%s'."), *TargetBoneName.ToString());
        return;
    }

    if (ProceduralMeshAttachSocketName.IsNone() || OtherHalfMeshAttachSocketName.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("SliceMeshAtBone: One or both socket names are invalid."));
        return;
    }

    ProcMeshComponent->SetSimulatePhysics(false);
    OtherHalfMesh->SetSimulatePhysics(false);

    FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
    ProcMeshComponent->AttachToComponent(GetMesh(), AttachRules, ProceduralMeshAttachSocketName);
    OtherHalfMesh->AttachToComponent(GetMesh(), AttachRules, OtherHalfMeshAttachSocketName);

    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->BreakConstraint(FVector(1000.f, 1000.f, 1000.f), FVector::ZeroVector, TargetBoneName);
    GetMesh()->SetSimulatePhysics(true);

    ProcMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}