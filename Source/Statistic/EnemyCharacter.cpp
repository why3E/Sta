#include "MyWindCutter.h"
#include "MyWindSkill.h"
#include "MyFireBall.h"
#include "MyFireSkill.h"
#include "EnemyCharacter.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/StaticMeshVertexBuffer.h"
#include "KismetProceduralMeshLibrary.h"
#include "Kismet/KismetMathLibrary.h"        // ⭐ 랜덤 벡터용 추가

#include "SESSION.h"

AEnemyCharacter::AEnemyCharacter()
{
    PrimaryActorTick.bCanEverTick = false;

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

    //// ⭐ 1초 뒤 자동 사망 (테스트용)
    //FTimerHandle TimerHandle;
    //GetWorld()->GetTimerManager().SetTimer(
    //    TimerHandle,
    //    this,
    //    &AEnemyCharacter::Die,
    //    1.0f,
    //    false
    //);

    //// ⭐ 3초 뒤 절단 테스트
    //FTimerHandle SliceTimerHandle;
    //GetWorld()->GetTimerManager().SetTimer(
    //    SliceTimerHandle,
    //    [this]()
    //    {
    //        FVector PlanePosition = ProcMeshComponent->GetComponentLocation() + FVector(0.f, 0.f, 30.f);    // 약간 위로
    //        FVector PlaneNormal = FVector(1.f, 0.f, 1.f).GetSafeNormal();   // 사선 절단
    //        SliceProcMesh(PlanePosition, PlaneNormal);
    //    },
    //    3.0f,
    //    false
    //);
}

void AEnemyCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
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

    if (g_is_host) {
        monster_attack_packet p;
        p.packet_size = sizeof(monster_attack_packet);
        p.packet_type = C2H_MONSTER_ATTACK_PACKET;
        p.monster_id = m_id;

        do_send(&p);
    }

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
    UE_LOG(LogTemp, Warning, TEXT("Damage: %f, HP: %f"), Info.Damage, HP);

    if (HP <= 0.f)
    {
        Die();
    }
}

void AEnemyCharacter::Die()
{
    GetMesh()->SetVisibility(false);
    CopySkeletalMeshToProcedural(0);
    ProcMeshComponent->SetVisibility(true);
    ProcMeshComponent->SetSimulatePhysics(true);

    FVector PlanePosition = ProcMeshComponent->GetComponentLocation() + FVector(0.f, 0.f, 30.f);    // 약간 위로
    FVector PlaneNormal = FVector(1.f, 0.f, 1.f).GetSafeNormal();   // 사선 절단
    SliceProcMesh(PlanePosition, PlaneNormal);
    UE_LOG(LogTemp, Warning, TEXT("Enemy died and switched to Procedural Mesh."));
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
        ProcMeshComponent->SetSimulatePhysics(true);
        ProcMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        ProcMeshComponent->SetEnableGravity(true);

        FVector RandomImpulse = UKismetMathLibrary::RandomUnitVector() * ImpulseStrength;
        ProcMeshComponent->AddImpulse(RandomImpulse, NAME_None, true);
    }

    if (OutOtherHalfProcMesh)
    {
        OutOtherHalfProcMesh->SetSimulatePhysics(true);
        OutOtherHalfProcMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        OutOtherHalfProcMesh->SetEnableGravity(true);

        FVector RandomImpulse = UKismetMathLibrary::RandomUnitVector() * ImpulseStrength;
        OutOtherHalfProcMesh->AddImpulse(RandomImpulse, NAME_None, true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Sliced Procedural Mesh with random impulses!"));
}

void AEnemyCharacter::Overlap(AActor* OtherActor) {
    AMySkillBase* Skill = Cast<AMySkillBase>(OtherActor);

    FSkillInfo Info;
    Info.Damage = Skill->GetDamage();
    Info.Element = Skill->GetElement();
    Info.StunTime = 1.5f;
    Info.KnockbackDir = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    ReceiveSkillHit(Info, Skill);
    UE_LOG(LogTemp, Warning, TEXT("Skill Hit to Monster %d"), get_id());
}

void AEnemyCharacter::do_send(void* buff) {
    EXP_OVER* o = new EXP_OVER;
    unsigned char packet_size = reinterpret_cast<unsigned char*>(buff)[0];
    memcpy(o->m_buffer, buff, packet_size);
    o->m_wsabuf[0].len = packet_size;

    DWORD send_bytes;
    auto ret = WSASend(g_h_socket, o->m_wsabuf, 1, &send_bytes, 0, &(o->m_over), send_callback);
    if (ret == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            delete o;
            return;
        }
    }
}