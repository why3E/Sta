#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimationAttackInterface.h"
#include "ReceiveDamageInterface.h"
#include "ProceduralMeshComponent.h"
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

protected:
    virtual void ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer) override;
    virtual void BaseAttackCheck() override;
    void CopySkeletalMeshToProcedural(int32 LODIndex);

private:
    bool bIsAttacking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats", Meta = (AllowPrivateAccess = "true"))
    float HP = 100.f;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    UAnimMontage* AttackMontage;
	
	UFUNCTION(BlueprintCallable, Category = "Slicing")
	void SliceProcMesh(FVector PlanePosition, FVector PlaneNormal);

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UProceduralMeshComponent* ProcMeshComponent;
};
