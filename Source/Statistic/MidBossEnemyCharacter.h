#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ImpactPointInterface.h"
#include "MyStoneWave.h"
#include "MyStoneSkill.h"
#include "MyWindCutter.h"
#include "MyWindLaser.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "ProceduralMeshComponent.h"
#include "MidBossEnemyCharacter.generated.h"

class UCapsuleComponent;

UCLASS()
class STATISTIC_API AMidBossEnemyCharacter : public ACharacter, public IImpactPointInterface
{
	GENERATED_BODY()

public:
	AMidBossEnemyCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 부위별 콜리전
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* HeadCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* ChestCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* HipCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* LeftArmUpperCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* LeftArmLowerCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* RightArmUpperCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* RightArmLowerCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* LeftLegUpperCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* LeftLegLowerCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* RightLegUpperCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Collision")
	UCapsuleComponent* RightLegLowerCollision;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* HitAttackMontage;

	void Attack();
	void SkillAttack();
	void PlayHitAttackMontage();

	FTimerHandle AttackTimerHandle;

	// IImpactPointInterface 구현
	virtual FVector GetCurrentImpactPoint() override;
	virtual FRotator GetCurrentImpactRot() override;
	virtual FVector GetFireLocation() override;

	FName BaseAttackSocketName;
	FName LaserAttackSocketName;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> StoneWaveClass;

	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> StoneSkillClass;

	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> WindCutterClass;

	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> WindSkillClass;

	UPROPERTY(VisibleAnywhere, Category = "Quiver", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> WindLaserClass;

public:
	TSubclassOf<AActor> GetStoneWaveClass() const { return StoneWaveClass; }
	TSubclassOf<AActor> GetStoneSkillClass() const { return StoneSkillClass; }
	TSubclassOf<AActor> GetWindCutterClass() const { return WindCutterClass; }
	TSubclassOf<AActor> GetWindSkillClass() const { return WindSkillClass; }
	TSubclassOf<AActor> GetWindLaserClass() const { return WindLaserClass; }

	void FindPlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	class APlayerCharacter* CachedPlayerCharacter = nullptr;

	UPROPERTY()
	AMyWindLaser* CurrentWindLaser = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	TArray<FVector> GenerateWindTonadoLocations(int32 Count, float MinRadius, float MaxRadius, float MinDistance);

	// 섹션 → 콜리전 맵
	UPROPERTY()
	TMap<FName, UCapsuleComponent*> MontageToHitCapsuleMap;

	UPROPERTY()
	bool bIsPlayingMontageSection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UNiagaraSystem* WeakPointEffect;

	UPROPERTY()
	UNiagaraComponent* ActiveWeakPointEffect;

	void SpawnWeakPointEffectForCurrentSection(FName SectionName);
	void RemoveWeakPointEffect();   

	// 충돌 처리 함수
	UFUNCTION()
	void OnHitCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                           bool bFromSweep, const FHitResult& SweepResult);


public:
	void Die();
    // 절단용 함수들
    void CopySkeletalMeshToProcedural(int32 LODIndex);
    void SliceMeshAtBone(FVector SliceNormal, bool bCreateOtherHalf);
	void ApplyVertexAlphaToSkeletalMesh();
    // 자를 본 이름 가져오기
    FName GetBoneName() const;

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

    UPROPERTY(EditAnywhere, Category="Slice")
	FName ProceduralMeshAttachSocketName = TEXT("SliceSocket_Upper");

	UPROPERTY(EditAnywhere, Category="Slice")
	FName OtherHalfMeshAttachSocketName = TEXT("SliceSocket_Lower");

    UPROPERTY(EditAnywhere)
    float CreateProceduralMeshDistance = 250.0f;

    UPROPERTY()
    UProceduralMeshComponent* CachedOtherHalfMesh = nullptr;

	UPROPERTY()
	UMaterialInterface* CapMaterial;

	int32 NumVertices;
};
