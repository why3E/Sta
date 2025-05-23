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

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!g_is_host) {
        if (get_is_attacking()) { return; }

        float Distance = FVector::Dist2D(GetActorLocation(), m_target_location);

        if (Distance < 100.0f) { return; }

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

void AEnemyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == AttackMontage)
    {
        bIsAttacking = false;
        OnAttackEnded.Broadcast();
    }
}

void AEnemyCharacter::MeleeAttack()
{
    if (bIsAttacking || !AttackMontage) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance)
    {
        float Duration = AnimInstance->Montage_Play(AttackMontage, 1.0f);
        if (Duration > 0.f)
        {
            bIsAttacking = true;
            FOnMontageEnded Delegate;
            Delegate.BindUObject(this, &AEnemyCharacter::OnAttackMontageEnded);
            AnimInstance->Montage_SetEndDelegate(Delegate, AttackMontage);
        }
    }
}

void AEnemyCharacter::ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer)
{
    HP -= Info.Damage;

    // 수정된 부분: 개별 팝업 출력
    ShowHud(Info.Damage, false);

    UE_LOG(LogTemp, Warning, TEXT("Damage: %f, HP: %f"), Info.Damage, HP);

    if (HP <= 0.f)
    {
        Die();
    }
}

void AEnemyCharacter::Die()
{
    // Remove Collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCanEverAffectNavigation(false);

    GetMesh()->SetVisibility(false);
    CopySkeletalMeshToProcedural(0);
    ProcMeshComponent->SetVisibility(true);
    ProcMeshComponent->SetSimulatePhysics(true);

    AAIController* AICon = Cast<AAIController>(GetController());
    if (AICon) {
        AICon->StopMovement();

        if (AICon->BrainComponent)
        {
            AICon->BrainComponent->StopLogic(TEXT("Character Died"));
        }
    }

    FVector PlanePosition = ProcMeshComponent->GetComponentLocation() + FVector(0.f, 0.f, 30.f);
    FVector PlaneNormal = FVector(1.f, 0.f, 1.f).GetSafeNormal();
    SliceProcMesh(PlanePosition, PlaneNormal);

    if (g_is_host) {
        GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AEnemyCharacter::Respawn, 10.0f, false);
    }
}

void AEnemyCharacter::Reset() {
    HP = 100.f;

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

void AEnemyCharacter::CopySkeletalMeshToProcedural(int32 LODIndex)
{
    if (!GetMesh() || !ProcMeshComponent) return;

    ProcMeshComponent->SetWorldLocation(GetMesh()->GetComponentLocation());
    ProcMeshComponent->SetWorldRotation(GetMesh()->GetComponentRotation());

    USkeletalMesh* SkeletalMesh = GetMesh()->GetSkeletalMeshAsset();
    if (!SkeletalMesh) return;

    const FSkeletalMeshRenderData* RenderData = SkeletalMesh->GetResourceForRendering();
    if (!RenderData || !RenderData->LODRenderData.IsValidIndex(LODIndex)) return;

    const FSkeletalMeshLODRenderData& LODRenderData = RenderData->LODRenderData[LODIndex];

    TArray<FVector> Vertices;
    TArray<int32> Indices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FColor> Colors;
    TArray<FProcMeshTangent> Tangents;

    for (const FSkelMeshRenderSection& Section : LODRenderData.RenderSections)
    {
        int32 Base = Section.BaseVertexIndex;
        int32 Count = Section.NumVertices;

        for (int32 i = 0; i < Count; i++)
        {
            int32 Idx = i + Base;
            FVector3f Pos = LODRenderData.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(Idx);
            Vertices.Add(FVector(Pos));

            FVector3f Normal = LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(Idx);
            Normals.Add(FVector(Normal));

            FVector3f Tangent = LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(Idx);
            Tangents.Add(FProcMeshTangent(FVector(Tangent), false));

            FVector2f UV = LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(Idx, 0);
            UVs.Add(FVector2D(UV));

            Colors.Add(FColor::White);
        }
    }

    const FRawStaticIndexBuffer16or32Interface* IndexBuffer = LODRenderData.MultiSizeIndexContainer.GetIndexBuffer();
    if (!IndexBuffer) return;

    int32 NumIndices = IndexBuffer->Num();
    Indices.SetNumUninitialized(NumIndices);
    for (int32 i = 0; i < NumIndices; i++)
        Indices[i] = static_cast<int32>(IndexBuffer->Get(i));

    ProcMeshComponent->CreateMeshSection(0, Vertices, Indices, Normals, UVs, Colors, Tangents, true);

    if (Vertices.Num() > 0)
    {
        ProcMeshComponent->ClearCollisionConvexMeshes();
        ProcMeshComponent->AddCollisionConvexMesh(Vertices);
    }

    ProcMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    ProcMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
    ProcMeshComponent->SetSimulatePhysics(true);
    ProcMeshComponent->SetEnableGravity(true);

    UMaterialInterface* Material = GetMesh()->GetMaterial(0);
    if (Material) ProcMeshComponent->SetMaterial(0, Material);
}

void AEnemyCharacter::SliceProcMesh(FVector PlanePosition, FVector PlaneNormal)
{
    if (!ProcMeshComponent) return;

    UProceduralMeshComponent* OutOtherHalfProcMesh = nullptr;
    UKismetProceduralMeshLibrary::SliceProceduralMesh(
        ProcMeshComponent,
        PlanePosition,
        PlaneNormal,
        true,
        OutOtherHalfProcMesh,
        EProcMeshSliceCapOption::CreateNewSectionForCap,
        nullptr
    );

    float ImpulseStrength = 500.f;

    if (ProcMeshComponent)
    {
        FVector RandomImpulse = UKismetMathLibrary::RandomUnitVector() * ImpulseStrength;
        ProcMeshComponent->AddImpulse(RandomImpulse, NAME_None, true);
    }

    if (OutOtherHalfProcMesh)
    {
        FVector RandomImpulse = UKismetMathLibrary::RandomUnitVector() * ImpulseStrength;
        OutOtherHalfProcMesh->AddImpulse(RandomImpulse, NAME_None, true);

        CachedOtherHalfMesh = OutOtherHalfProcMesh;
    }

    UE_LOG(LogTemp, Warning, TEXT("Sliced Procedural Mesh with random impulses!"));
}

void AEnemyCharacter::Overlap(AActor* OtherActor)
{
    AMySkillBase* Skill = Cast<AMySkillBase>(OtherActor);

    FSkillInfo Info;
    Info.Damage = Skill->GetDamage();
    Info.Element = Skill->GetElement();
    Info.StunTime = 1.5f;
    Info.KnockbackDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    ReceiveSkillHit(Info, Skill);
    UE_LOG(LogTemp, Warning, TEXT("Skill Hit to Monster %d"), get_id());
}

void AEnemyCharacter::ShowHud(float Damage, bool bIsCritical)
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
        popupActor->InitDamage(Damage, bIsCritical);
        UE_LOG(LogTemp, Warning, TEXT("Damage Popup Actor Spawned"));
    }
}
