#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AnimationAttackInterface.h"
#include "ReceiveDamageInterface.h"
#include "ProceduralMeshComponent.h"
#include "DamageWidget.h" // UDamageWidget 헤더 추가
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

    void set_hp(float hp) { HP = hp; }
    float get_hp() { return HP; }
    bool get_is_attacking() { return bIsAttacking; }

protected:
    UPROPERTY(VisibleAnywhere, Category = "Components")
    UProceduralMeshComponent* ProcMeshComponent;

public:
    unsigned short m_id;
    FVector m_target_location;
    FRotator m_target_rotation;

public:
    virtual void Overlap(AActor* OtherActor);

    unsigned short get_id() { return m_id; }
    void set_id(unsigned short id) { m_id = id; }
    void set_target_location(FVector target_location) { m_target_location = target_location; }
    void set_target_rotation(FRotator target_rotation) { m_target_rotation = target_rotation; }

    void do_send(void* buff);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hud")
    TSubclassOf<class ADamagePopupActor> DamagePopupActorClass;

    UFUNCTION(BlueprintCallable, Category = "Hud")
    void ShowHud(float Damage, bool bIsCritical = false);
};
