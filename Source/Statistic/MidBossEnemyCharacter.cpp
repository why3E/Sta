#include "MidBossEnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerCharacter.h"
#include "MyStoneWave.h"
#include "MyStoneSkill.h"
#include "PlayerCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

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
#include "Components/WidgetComponent.h" 
#include "Enums.h"
#include "DamagePopupActor.h"
#include "UObject/ConstructorHelpers.h"
#include "NiagaraFunctionLibrary.h"

AMidBossEnemyCharacter::AMidBossEnemyCharacter()
{
    MaxHP = 100.0f;
    HP = MaxHP;

    m_view_radius = 2000.0f;
    m_track_radius = 3000.0f;
    m_wander_radius = 1000.0f;
    m_attack_radius = 2000.0f;

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

    hpFloatingWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("floatingWidget"));
    hpFloatingWidget->SetupAttachment(RootComponent);
    hpFloatingWidget->SetRelativeLocation(FVector(0, 0, 125));
    hpFloatingWidget->SetWorldScale3D(FVector(1.0, 0.23, 0.03));
    hpFloatingWidget->SetWidgetSpace(EWidgetSpace::Screen);
}

void AMidBossEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
    MonsterHpBarWidget = Cast<UMonsterHPBarWidget>(hpFloatingWidget->GetUserWidgetObject());

    HeadCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);
    ChestCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);
    HipCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);

    LeftArmUpperCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);
    LeftArmLowerCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);

    RightArmUpperCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);
    RightArmLowerCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);

    LeftLegUpperCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);
    LeftLegLowerCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);

    RightLegUpperCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);
    RightLegLowerCollision->OnComponentBeginOverlap.AddDynamic(this, &AMidBossEnemyCharacter::OnHitCollisionOverlap);

	MontageToHitCapsuleMap.Add(TEXT("WindLaser"), RightArmLowerCollision);
	MontageToHitCapsuleMap.Add(TEXT("WindCutter"), LeftArmLowerCollision);
	MontageToHitCapsuleMap.Add(TEXT("StoneWave"), ChestCollision);
	MontageToHitCapsuleMap.Add(TEXT("StoneThrow"), HipCollision);
	MontageToHitCapsuleMap.Add(TEXT("WindTonado"), HipCollision);



}

void AMidBossEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (m_is_rotating) {
        rotate_to_target(DeltaTime);
    }

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

    if(MonsterHpBarWidget) {
        MonsterHpBarWidget->updateHpBar(HP, MaxHP);
        // 체력이 100이 아닐 때만 HP바 보이기
        if (HP < MaxHP) {
            hpFloatingWidget->SetVisibility(true);
        } else {
            hpFloatingWidget->SetVisibility(false);
        }
    }
}

void AMidBossEnemyCharacter::rotate_to_target(float DeltaTime) {
    FVector direction = (m_skill_location - GetActorLocation()).GetSafeNormal2D();

    // Rotate
    FRotator target_rotation = FRotator(0.0f, direction.Rotation().Yaw, 0.0f);
    FRotator current_rotation = GetActorRotation();

    FRotator NewRotation = FMath::RInterpTo(current_rotation, target_rotation, DeltaTime, 5.0f);
    SetActorRotation(NewRotation);

    float angle_diff = FMath::Abs(FRotator::NormalizeAxis(current_rotation.Yaw - target_rotation.Yaw));

    if (angle_diff < 5.0f) {
        m_is_rotating = false;
    }
}

void AMidBossEnemyCharacter::start_attack(AttackType attack_type) {

}

void AMidBossEnemyCharacter::start_attack(AttackType attack_type, FVector attack_location) {
    m_is_rotating = true;

    // StoneWave일 때 바라보는 방향(Forward Vector)으로 600만큼 더함
    if (attack_type == AttackType::StoneWave) {
        FVector Forward = GetActorForwardVector();
        attack_location += Forward * 600.f;
    }

    m_skill_location = attack_location;

    Attack(attack_type);
}

void AMidBossEnemyCharacter::Attack(AttackType attack_type)
{
    switch (attack_type) {
    case AttackType::WindCutter:
    case AttackType::WindLaser:
    case AttackType::StoneWave:
        if (AttackMontage) {
            UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
            if (AnimInstance) {
                AnimInstance->OnMontageEnded.RemoveDynamic(this, &AMidBossEnemyCharacter::OnAttackMontageEnded);
                AnimInstance->OnMontageEnded.AddDynamic(this, &AMidBossEnemyCharacter::OnAttackMontageEnded);

                int32 SectionIndex = static_cast<int32>(attack_type) - 1;
                FName SelectedSection = Sections[SectionIndex];

                float PlayRate = (SelectedSection == TEXT("WindLaser")) ? 0.5f : 1.0f;

                bIsPlayingMontageSection = true; 
                AnimInstance->Montage_Play(AttackMontage, PlayRate);
                AnimInstance->Montage_JumpToSection(SelectedSection, AttackMontage);
                SpawnWeakPointEffectForCurrentSection(SelectedSection);
            }
        }
        break;

    case AttackType::WindTornado:
    case AttackType::StoneSkill:
        if (AttackMontage) {
            UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->OnMontageEnded.RemoveDynamic(this, &AMidBossEnemyCharacter::OnAttackMontageEnded);
                AnimInstance->OnMontageEnded.AddDynamic(this, &AMidBossEnemyCharacter::OnAttackMontageEnded);

                int32 SectionIndex = static_cast<int32>(attack_type) - 1;
                FName SelectedSection = Sections[SectionIndex];

                bIsPlayingMontageSection = true; 
                AnimInstance->Montage_Play(AttackMontage, 0.5f);
                AnimInstance->Montage_JumpToSection(SelectedSection, AttackMontage);
                SpawnWeakPointEffectForCurrentSection(SelectedSection);
            }
        }
        break;
    }
}

void AMidBossEnemyCharacter::PlayHitAttackMontage() {
    if (HitAttackMontage) {
        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

        if (AnimInstance) {
            AnimInstance->OnMontageEnded.RemoveDynamic(this, &AMidBossEnemyCharacter::OnStunMontageEnded);
            AnimInstance->OnMontageEnded.AddDynamic(this, &AMidBossEnemyCharacter::OnStunMontageEnded);

            AnimInstance->Montage_Play(HitAttackMontage, 1.0f);

            RemoveWeakPointEffect();
        }
    }
}

void AMidBossEnemyCharacter::FindPlayerCharacter()
{
    TArray<AActor*> PlayerActors;

    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerCharacter::StaticClass(), PlayerActors);

    float closest_dist = m_attack_radius;
    APlayerCharacter* closest_player = nullptr;

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (g_c_players[i]) {
            APlayerCharacter* player = Cast<APlayerCharacter>(g_c_players[i]);

            if (player != nullptr) {
                float dist = (player->GetActorLocation() - GetActorLocation()).Size2D();

                if ((dist < m_attack_radius) && (dist < closest_dist)) {
                    closest_dist = dist;
                    closest_player = player;
                }
            }
        }
    }

    CachedPlayerCharacter = closest_player ? closest_player : nullptr;
}

void AMidBossEnemyCharacter::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted) {
    OnAttackEnded.Broadcast();
}

void AMidBossEnemyCharacter::OnStunMontageEnded(UAnimMontage* Montage, bool bInterrupted) {
    if (Montage == HitAttackMontage) {
        if (AAIController* AICon = Cast<AAIController>(GetController())) {
            if (UBlackboardComponent* BB = AICon->GetBlackboardComponent()) {
                BB->SetValueAsBool(TEXT("bIsStunned"), false);

                OnAttackEnded.Broadcast();
            }
        }

        UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

        if (AnimInstance) {
            AnimInstance->OnMontageEnded.RemoveDynamic(this, &AMidBossEnemyCharacter::OnStunMontageEnded);
        }
    }
}

FVector AMidBossEnemyCharacter::GetFireLocation() {
    return m_skill_location; 
}

FVector AMidBossEnemyCharacter::GetCurrentImpactPoint() { 
    return m_skill_location;
}

FRotator AMidBossEnemyCharacter::GetCurrentImpactRot() { 
    return (m_skill_location - GetActorLocation()).GetSafeNormal().Rotation();
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

TArray<FVector> AMidBossEnemyCharacter::GenerateWindTonadoLocations(int32 Count, float MinRadius, float MaxRadius, float MinDistance) {
    TArray<FVector> Result;
    FVector Center = m_skill_location;
    float Radius = 300.0f;

    for (int32 i = 0; i < 3; ++i) {
        float AngleDeg = i * 120.0f; 
        float AngleRad = FMath::DegreesToRadians(AngleDeg);
        FVector Offset = FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.f) * Radius;

        Result.Add(Center + Offset);
    }

    return Result;
}

void AMidBossEnemyCharacter::Die()
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (AnimInstance && AnimInstance->Montage_IsPlaying(AttackMontage))
    {
        AnimInstance->Montage_Stop(0.1f, AttackMontage);
    }
    if (AnimInstance && AnimInstance->Montage_IsPlaying(HitAttackMontage))
    {
        AnimInstance->Montage_Stop(0.1f, HitAttackMontage);
    }
    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
        if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AICon->BrainComponent))
        {
            BTComp->StopTree(EBTStopMode::Safe);
        }
    }

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

void AMidBossEnemyCharacter::Reset() {

}

void AMidBossEnemyCharacter::Respawn() {

}

void AMidBossEnemyCharacter::Respawn(FVector respawn_location) {

}

bool bIsProcessingHit = false;

void AMidBossEnemyCharacter::OnHitCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
    if (bIsProcessingHit) return; // 재진입 방지
    bIsProcessingHit = true;

    if (g_is_host) {
        if (OtherActor && (OtherActor->GetOwner() != this)) {
            UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

            if (!AnimInstance || !AnimInstance->Montage_IsPlaying(AttackMontage)) { bIsProcessingHit = false; return; }

            FName CurrentSection = AnimInstance->Montage_GetCurrentSection(AttackMontage);

            if (CurrentSection.IsNone()) { bIsProcessingHit = false; return; }

            if (MontageToHitCapsuleMap.Contains(CurrentSection) && MontageToHitCapsuleMap[CurrentSection] == OverlappedComp) {
                {
                    MonsterEvent monster_event = DamagedEvent(m_id);
                    std::lock_guard<std::mutex> lock(g_s_monster_events_l);
                    g_s_monster_events.push(monster_event);
                }

                PlayStunMontage();
            }
        }
    }

    bIsProcessingHit = false;
}
    


void AMidBossEnemyCharacter::Overlap(char skill_type, FVector skill_location) {
    HP -= 10.0f;

    switch (skill_type) {
    case SKILL_WIND_CUTTER:
    case SKILL_WIND_TORNADO:
        ShowHud(10, EClassType::CT_Wind);
        break;

    case SKILL_FIRE_BALL:
    case SKILL_FIRE_WALL:
        ShowHud(10, EClassType::CT_Fire);
        break;

    case SKILL_STONE_WAVE:
    case SKILL_STONE_SKILL:
        ShowHud(10, EClassType::CT_Stone);
        break;

    case SKILL_ICE_ARROW:
    case SKILL_ICE_WALL:
        ShowHud(10, EClassType::CT_Ice);
        break;
    }
}

void AMidBossEnemyCharacter::PlayStunMontage() {
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

    if (g_is_host) {
        AAIController* AICon = Cast<AAIController>(GetController());

        if (AICon) {
            UBlackboardComponent* BB = AICon->GetBlackboardComponent();

            if (BB) {
                BB->SetValueAsBool(TEXT("bIsStunned"), true);
            }
        }
    }

    AnimInstance->OnMontageEnded.RemoveDynamic(this, &AMidBossEnemyCharacter::OnAttackMontageEnded);
    AnimInstance->Montage_Stop(0.1f, AttackMontage);

    PlayHitAttackMontage();

    bIsPlayingMontageSection = false; 
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

    CapMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Materials/M_CutFace.M_CutFace"));
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

FName AMidBossEnemyCharacter::GetBoneName() const
{
    return TEXT("spine_03");
}

FName AMidBossEnemyCharacter::GetSecondBoneName() const
{
    return TEXT("spine_04");
}

void AMidBossEnemyCharacter::ShowHud(float Damage, EClassType Type)
{
    if (!DamagePopupActorClass)
    {
        return;
    }

    FVector spawnLoc = GetActorLocation() + FVector(0.f, 0.f, 1000.f);

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

void AMidBossEnemyCharacter::ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer)
{
    HP -= Info.Damage;

    ShowHud(Info.Damage, Info.Element);

    UE_LOG(LogTemp, Warning, TEXT("Damage: %f, HP: %f"), Info.Damage, HP);

    if (HP <= 0.0f) {
        Die();
    }
}