#pragma once

#include "MyEnemyBase.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimationAttackInterface.h"
#include "ReceiveDamageInterface.h"
#include "ProceduralMeshComponent.h"
#include "DamageWidget.h" // UDamageWidget 헤더 추가
#include "Enums.h"
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
    FName ProceduralMeshAttachSocketName;

    UPROPERTY(EditAnywhere)
    FName OtherHalfMeshAttachSocketName;

    UPROPERTY(EditAnywhere)
    float CreateProceduralMeshDistance = 50.0f;

    UPROPERTY()
    UProceduralMeshComponent* CachedOtherHalfMesh = nullptr;

    void CopySkeletalMeshToProcedural(int32 LODIndex);
    void SliceProcMesh(FVector PlaneNormal);

    FName GetSecondBoneName() const;

    UPROPERTY(EditAnywhere, Category = "MySettings")
    class UWidgetComponent* hpFloatingWidget;
};
