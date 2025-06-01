#include "EnemyCharacter.h"
#include "MyWindCutter.h"
#include "MyWindSkill.h"
#include "MyFireBall.h"
#include "MyFireSkill.h"
#include "AIController.h"
#include "BrainComponent.h"

#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

#include "DamageWidget.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/StaticMeshVertexBuffer.h"
#include "KismetProceduralMeshLibrary.h"
#include "Kismet/KismetMathLibrary.h"        
#include "Components/WidgetComponent.h" 
#include "DamagePopupActor.h"

#include "SESSION.h"

AEnemyCharacter::AEnemyCharacter()
{
    MaxHP = 100.0f;
    HP = MaxHP;

    m_view_radius = 500.0f;
    m_track_radius = 1000.0f;
    m_wander_radius = 500.0f;
    m_attack_radius = 100.0f;

    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.f, 50.f);

    GetCharacterMovement()->JumpZVelocity = 700.f;
    GetCharacterMovement()->AirControl = 0.35f;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;
    GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
    GetCharacterMovement()->BrakingDecelerationFalling = 1500.f;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    ProcMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProcMeshComponent"));
    ProcMeshComponent->SetupAttachment(RootComponent);
    ProcMeshComponent->SetVisibility(false);
    ProcMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyCharacter::BeginPlay()
{
    Super::BeginPlay();
    bIsAttacking = false;

    UE_LOG(LogTemp, Warning, TEXT("Slime Position: %s"), *GetActorLocation().ToString());
}

void AEnemyCharacter::Tick(float DeltaTime) {
    Super::Tick(DeltaTime);

    if (!g_is_host) {
        if ((m_target_location - GetActorLocation()).Size2D() < 100.0f) {
            m_target_location = GetActorLocation();
            return; 
        }

        FVector Direction = (m_target_location - GetActorLocation()).GetSafeNormal2D();

        // Rotate
        FRotator TargetRotation = Direction.Rotation();
        FRotator CurrentRotation = GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f);

        SetActorRotation(NewRotation);

        // Move
        AddMovementInput(Direction, 1.0f);
    }
}

void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyCharacter::start_attack(AttackType attack_type) {
    if (bIsAttacking || !AttackMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    if (AnimInstance) {
        float Duration = AnimInstance->Montage_Play(AttackMontage, 1.0f);

        if (Duration > 0.f) {
            bIsAttacking = true;
            FOnMontageEnded Delegate;
            Delegate.BindUObject(this, &AEnemyCharacter::OnAttackMontageEnded);
            AnimInstance->Montage_SetEndDelegate(Delegate, AttackMontage);
        }
    }
}

void AEnemyCharacter::start_attack(AttackType attack_type, FVector attack_location) {

}

void AEnemyCharacter::BaseAttackCheck()
{
    TArray<FHitResult> Hits;
    float Range = 100.f;
    float Radius = 50.f;
    FVector Start = GetActorLocation() + (GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius());
    FVector End = Start + (GetActorForwardVector() * Range);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);
    bool bHit = GetWorld()->SweepMultiByChannel(
        Hits, Start, End, FQuat::Identity,
        ECC_GameTraceChannel2, FCollisionShape::MakeSphere(Radius), Params);

    FVector CapsuleCenter = Start + (End - Start) * 0.5f;
    float HalfHeight = Range * 0.5f;
    FColor Color = bHit ? FColor::Green : FColor::Red;

    DrawDebugCapsule(GetWorld(), CapsuleCenter, HalfHeight, Radius,
        FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(),
        Color, false, 3.f);
}

void AEnemyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == AttackMontage)
    {
        bIsAttacking = false;
        OnAttackEnded.Broadcast();
    }
}

void AEnemyCharacter::ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer)
{
    HP -= Info.Damage;

    ShowHud(Info.Damage, Info.Element);

    UE_LOG(LogTemp, Warning, TEXT("Damage: %f, HP: %f"), Info.Damage, HP);

    if (HP <= 0.0f) {
        Die();
    }
}

void AEnemyCharacter::Die()
{
    // Remove Collision
    
    CopySkeletalMeshToProcedural(0);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCanEverAffectNavigation(false);

    GetMesh()->SetVisibility(false);
    
    ProcMeshComponent->SetVisibility(true);
    ProcMeshComponent->SetSimulatePhysics(true);

    // 슬라이스 실행
    FVector PlaneNormal = FVector(1.f, 0.f, 1.f).GetSafeNormal();
    SliceProcMesh(PlaneNormal);

    AAIController* AICon = Cast<AAIController>(GetController());
    if (AICon) {
        AICon->StopMovement();

        if (AICon->BrainComponent) {
            AICon->BrainComponent->StopLogic(TEXT("Character Died"));
        }
    }

    //if (g_is_host) {
    //    GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AEnemyCharacter::Respawn, 2.5f, false);
    //}
}

void AEnemyCharacter::Reset() {
    HP = MaxHP;

    // Reset Collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetCapsuleComponent()->SetCanEverAffectNavigation(true);

    // Reset Mesh
    GetMesh()->SetVisibility(true);

    ProcMeshComponent->SetVisibility(false);
    ProcMeshComponent->SetSimulatePhysics(false);
    ProcMeshComponent->ClearAllMeshSections();

    if (CachedOtherHalfMesh) {
        CachedOtherHalfMesh->DestroyComponent();
        CachedOtherHalfMesh = nullptr;
    }
}

void AEnemyCharacter::Respawn() {
    Reset();

    AAIController* AICon = Cast<AAIController>(GetController());

    UBlackboardComponent* BB = AICon->GetBlackboardComponent();

    SetActorLocation(BB->GetValueAsVector(TEXT("StartLocation")));

    UBehaviorTree* BTAsset = LoadObject<UBehaviorTree>(
        nullptr,
        TEXT("/Game/Slime/AI/BT_EnemyAI.BT_EnemyAI")
    );

    AICon->RunBehaviorTree(BTAsset);

    {
        MonsterEvent monster_event = RespawnEvent(m_id, BB->GetValueAsVector(TEXT("StartLocation")));
        std::lock_guard<std::mutex> lock(g_s_monster_events_l);
        g_s_monster_events.push(monster_event);
    }
}

void AEnemyCharacter::Respawn(FVector respawn_location) {
    Reset();

    SetActorLocation(respawn_location);
}

void AEnemyCharacter::Overlap(char skill_type, FVector skill_location) {
    FSkillInfo Info;
    Info.Damage = 10.0f;

    switch (skill_type) {
    case SKILL_WIND_CUTTER:
    case SKILL_WIND_TORNADO:
        Info.Element = EClassType::CT_Wind;
        break;

    case SKILL_FIRE_BALL:
    case SKILL_FIRE_WALL:
        Info.Element = EClassType::CT_Fire;
        break;

    case SKILL_STONE_WAVE:
    case SKILL_STONE_SKILL:
        Info.Element = EClassType::CT_Stone;
        break;

    case SKILL_ICE_ARROW:
    case SKILL_ICE_WALL:
        Info.Element = EClassType::CT_Ice;
        break;
    }

    Info.StunTime = 1.5f;
    Info.KnockbackDir = (skill_location - GetActorLocation()).GetSafeNormal();

    ReceiveSkillHit(Info, nullptr);
}

void AEnemyCharacter::ShowHud(float Damage, EClassType Type)
{
    if (!DamagePopupActorClass)
    {
        return;
    }

    FVector spawnLoc = GetActorLocation() + FVector(0.f, 0.f, 140.f);

    // X, Y에 랜덤 흔들림 추가
    spawnLoc.X += FMath::RandRange(-80.f, 80.f);
    spawnLoc.Y += FMath::RandRange(-80.f, 80.f);

    FRotator spawnRot = FRotator::ZeroRotator;

    ADamagePopupActor* popupActor = GetWorld()->SpawnActor<ADamagePopupActor>(DamagePopupActorClass, spawnLoc, spawnRot);
    if (popupActor)
    {
        popupActor->InitDamage(Damage, Type);
        UE_LOG(LogTemp, Warning, TEXT("Damage Popup Actor Spawned"));
    }
}

void AEnemyCharacter::CopySkeletalMeshToProcedural(int32 LODIndex)
{
    TargetBoneName = GetSecondBoneName();
    if (!GetMesh() || !ProcMeshComponent || TargetBoneName.IsNone()) return;

    ProcMeshComponent->ClearAllMeshSections();
    FilteredVerticesArray.Reset();
    Indices.Reset();
    Normals.Reset();
    UV.Reset();
    Colors.Reset();
    Tangents.Reset();
    VertexIndexMap.Reset();

    FVector MeshLocation = GetMesh()->GetComponentLocation();
    FRotator MeshRotation = GetMesh()->GetComponentRotation();
    ProcMeshComponent->SetWorldLocation(MeshLocation);
    ProcMeshComponent->SetWorldRotation(MeshRotation);

    USkeletalMesh* SkeletalMesh = GetMesh()->GetSkeletalMeshAsset();
    if (!SkeletalMesh) return;

    const FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();
    if (!RenderData || !RenderData->LODRenderData.IsValidIndex(LODIndex)) return;

    const FSkeletalMeshLODRenderData& LODRenderData = RenderData->LODRenderData[LODIndex];
    FTransform MeshTransform = GetMesh()->GetComponentTransform();
    FVector TargetBoneLocation = GetMesh()->GetBoneLocation(TargetBoneName);

    int32 vertexCounter = 0;
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
                FVector LocalVertexPosition = FVector(SkinnedVectorPos);
                VertexIndexMap.Add(VertexIndex, vertexCounter);
                FilteredVerticesArray.Add(LocalVertexPosition);
                vertexCounter += 1;

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

    const FRawStaticIndexBuffer16or32Interface* IndexBuffer = LODRenderData.MultiSizeIndexContainer.GetIndexBuffer();
    if (!IndexBuffer) return;

    const int32 NumIndices = IndexBuffer->Num();
    for (int32 i = 0; i < NumIndices; i += 3)
    {
        int32 OldIndex0 = static_cast<int32>(IndexBuffer->Get(i));
        int32 OldIndex1 = static_cast<int32>(IndexBuffer->Get(i + 1));
        int32 OldIndex2 = static_cast<int32>(IndexBuffer->Get(i + 2));

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
    ProcMeshComponent->ClearCollisionConvexMeshes();
    ProcMeshComponent->AddCollisionConvexMesh(FilteredVerticesArray);

    ProcMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ProcMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    ProcMeshComponent->SetSimulatePhysics(false);
    ProcMeshComponent->SetEnableGravity(true);

    UMaterialInterface* SkeletalMeshMaterial = GetMesh()->GetMaterial(0);
    if (SkeletalMeshMaterial)
        ProcMeshComponent->SetMaterial(0, SkeletalMeshMaterial);
}

void AEnemyCharacter::SliceProcMesh(FVector PlaneNormal)
{
    // 자를 본 이름 자동 설정
    TargetBoneName = GetSecondBoneName();
    if (!GetMesh() || !ProcMeshComponent || TargetBoneName.IsNone()) return;

    // 절단 기준 위치
    FVector PlanePosition = GetMesh()->GetBoneLocation(TargetBoneName);

    // 절단면 머티리얼 로드
    UMaterialInterface* CapMaterial = LoadObject<UMaterialInterface>(
        nullptr, TEXT("/Game/Materials/M_CutFace.M_CutFace")); // 경로는 본인 머티리얼에 맞게 수정

    UProceduralMeshComponent* OtherHalfMesh = nullptr;

    // 절단 수행
    UKismetProceduralMeshLibrary::SliceProceduralMesh(
        ProcMeshComponent,
        PlanePosition,
        PlaneNormal,
        true,                            // bCreateOtherHalf
        OtherHalfMesh,
        EProcMeshSliceCapOption::CreateNewSectionForCap,
        CapMaterial
    );

    if (!OtherHalfMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("SliceProcMesh: Failed to slice at bone %s"), *TargetBoneName.ToString());
        return;
    }

    // 반드시 등록! 그렇지 않으면 월드에 안 보임
    OtherHalfMesh->RegisterComponent();

    // 메시 겹침 방지 - PlaneNormal 방향으로 살짝 밀어줌
    FVector Offset = PlaneNormal * 2.0f;
    OtherHalfMesh->AddLocalOffset(Offset);

    // 소켓에 부착 (필요한 경우)
    FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
    ProcMeshComponent->AttachToComponent(GetMesh(), TransformRules, ProceduralMeshAttachSocketName);
    OtherHalfMesh->AttachToComponent(GetMesh(), TransformRules, OtherHalfMeshAttachSocketName);

    // 메시 충돌 설정
    ProcMeshComponent->SetSimulatePhysics(false);
    OtherHalfMesh->SetSimulatePhysics(false);
    ProcMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    OtherHalfMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // 절단된 뼈에 물리 적용
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->BreakConstraint(FVector(0.f, 0.f, 0.f), FVector::ZeroVector, TargetBoneName);
    GetMesh()->SetSimulatePhysics(true);

    // 튕김 방지를 위해 물리 적용은 0.2초 후에 딜레이
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [OtherHalfMesh]()
    {
        if (OtherHalfMesh)
        {
            OtherHalfMesh->SetSimulatePhysics(true);
        }
    }, 0.2f, false);

    // 나중에 제거 위해 저장
    CachedOtherHalfMesh = OtherHalfMesh;
}

FName AEnemyCharacter::GetSecondBoneName() const
{
    if (!GetMesh()) return NAME_None;

    const int32 NumBones = GetMesh()->GetNumBones();
    if (NumBones < 2) return NAME_None;

    FName BoneName = GetMesh()->GetBoneName(1);
    UE_LOG(LogTemp, Warning, TEXT("Second Bone Name: %s"), *BoneName.ToString());
    return BoneName;
}
