#pragma once

#include "MyEnemyBase.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimationAttackInterface.h"
#include "ReceiveDamageInterface.h"
#include "ProceduralMeshComponent.h"
#include "DamageWidget.h" // UDamageWidget 헤더 추가
#include "Enums.h"
#include "MonsterHPBarWidget.h"
#include "MyItemDropActor.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class STATISTIC_API AEnemyCharacter : public AMyEnemyBase, public IReceiveDamageInterface, public IAnimationAttackInterface {
    GENERATED_BODY()

public:
    AEnemyCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    // Combat
    bool bIsAttacking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* AttackMontage;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);
    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnAttackEnded OnAttackEnded;

    virtual void start_attack(AttackType attack_type) override;
    virtual void start_attack(AttackType attack_type, FVector attack_location) override;

    virtual void BaseAttackCheck() override;
    virtual void FarAttackCheck() override;

    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    virtual void ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer) override;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool GetIsAttacking() const { return bIsAttacking; }
    
public:
    // Die
    virtual void Die() override;
    virtual void Reset() override;
    virtual void Respawn() override;
    virtual void Respawn(FVector respawn_location) override;

public:
    // Collision
    virtual void Overlap(char skill_type, FVector skill_location) override;

public:
    // Hud
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hud")
    TSubclassOf<class ADamagePopupActor> DamagePopupActorClass;

    UFUNCTION(BlueprintCallable, Category = "Hud")
    void ShowHud(float Damage, EClassType Type);

public:
    // ProceduralMesh
    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* ProcMeshComponent;

    TArray<FVector> FilteredVerticesArray;
    TArray<int32> Indices;
    TArray<FVector> Normals;
    TArray<FVector2D> UV;
    TArray<FColor> Colors;
    TArray<FProcMeshTangent> Tangents;
    TMap<int32, int32> VertexIndexMap;

    UPROPERTY(EditAnywhere)
    FName TargetBoneName;

    UPROPERTY(EditAnywhere)
    FName ProceduralMeshAttachSocketName = "Bonesocket";

    UPROPERTY(EditAnywhere)
    FName OtherHalfMeshAttachSocketName = "Bone_002socket";

    UPROPERTY(EditAnywhere)
    float CreateProceduralMeshDistance = 50.0f;

    UPROPERTY()
    UProceduralMeshComponent* CachedOtherHalfMesh = nullptr;

    void CopySkeletalMeshToProcedural(int32 LODIndex);
    void SliceProcMesh(FVector PlaneNormal);

    FName GetSecondBoneName() const;

    UPROPERTY(EditAnywhere, Category = "MySettings")
    class UWidgetComponent* hpFloatingWidget;

    class UMonsterHPBarWidget* MonsterHpBarWidget;

    

protected:
    // 드랍된 아이템 액터를 저장할 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class AMyItemDropActor> DroppedItemActorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatMontage")
    FName AttackNMontageSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatMontage")
    FName AttackFMontageSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatMontage")
    FName DieMontageSection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CombatMontage")
    FName HitMontageSection;

    float LastAttackNTime = -FLT_MAX;
    float LastAttackFTime = -FLT_MAX;
    float LastHitTime     = -FLT_MAX;

    // 각 애니메이션 별 쿨타임 (초 단위)
    float CooldownAttackN = 2.0f;
    float CooldownAttackF = 5.0f;
    float CooldownHit     = 1.0f;

    UFUNCTION()
    void OnHitMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsRangeHit = false;

    // 애니메이션 쿨타임 관리용
    float LastAttackTime = -FLT_MAX;
    float CooldownCurrentAttack = 0.0f;

private:
    UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> WindCutterClass;

	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> FireBallClass;

	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> IceArrowClass;

};
