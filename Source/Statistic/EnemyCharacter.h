#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimationAttackInterface.h"
#include "ReceiveDamageInterface.h"
#include "ProceduralMeshComponent.h"
#include "DamageWidget.h" // UDamageWidget 헤더 추가
#include "Enums.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class STATISTIC_API AEnemyCharacter : public ACharacter, public IReceiveDamageInterface, public IAnimationAttackInterface
{
    GENERATED_BODY()

public:
    AEnemyCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // 공격
    void MeleeAttack();
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool GetIsAttacking() const { return bIsAttacking; }
    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);
    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnAttackEnded OnAttackEnded;

    // 사망 함수
    UFUNCTION()
    void Die();

    void Reset();
    void Respawn();
    void Respawn(FVector respawn_location);

protected:
    virtual void ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer) override;
    virtual void BaseAttackCheck() override;

private:
    bool bIsAttacking;
    FTimerHandle RespawnTimerHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Meta = (AllowPrivateAccess = "true"))
    float HP = 100.0f;
    float MaxHP = 100.0f;

    UPROPERTY()
    UProceduralMeshComponent* CachedOtherHalfMesh = nullptr;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* AttackMontage;

    void set_hp(float hp) { HP = hp; }
    float get_hp() { return HP; }
    bool get_is_attacking() { return bIsAttacking; }

public:
    unsigned short m_id;
    FVector m_target_location;

public:
    virtual void Overlap(char skill_type, FVector skill_location);

    unsigned short get_id() { return m_id; }
    FVector get_target_location() { return m_target_location; }

    void set_id(unsigned short id) { m_id = id; }
    void set_target_location(FVector target_location) { m_target_location = target_location; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hud")
    TSubclassOf<class ADamagePopupActor> DamagePopupActorClass;

    UFUNCTION(BlueprintCallable, Category = "Hud")
    void ShowHud(float Damage, EClassType Type);

public:
    // 절단용 함수들
    void CopySkeletalMeshToProcedural(int32 LODIndex);
    void SliceProcMesh(FVector PlaneNormal);

    // 자를 본 이름 가져오기
    FName GetSecondBoneName() const;

private:
    // ProceduralMesh용 데이터
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
    FName ProceduralMeshAttachSocketName;

    UPROPERTY(EditAnywhere)
    FName OtherHalfMeshAttachSocketName;

    UPROPERTY(EditAnywhere)
    float CreateProceduralMeshDistance = 50.0f;


};
