#pragma once

#include "MyEnemyBase.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ImpactPointInterface.h"
#include "MyStoneWave.h"
#include "MyStoneSkill.h"
#include "MyWindCutter.h"
#include "MyWindLaser.h"
#include "PlayerCharacter.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "ProceduralMeshComponent.h"
#include "MonsterHPBarWidget.h"
#include "Components/WidgetComponent.h" 
#include "MidBossEnemyCharacter.generated.h"

class UCapsuleComponent;

UCLASS()
class STATISTIC_API AMidBossEnemyCharacter : public AMyEnemyBase, public IImpactPointInterface, public IReceiveDamageInterface{
	GENERATED_BODY()

public:
	AMidBossEnemyCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
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
	// Combat
	FName BaseAttackSocketName;
	FName LaserAttackSocketName;

	FTimerHandle AttackTimerHandle;
	
	TArray<FName> Sections = { TEXT("WindCutter"), TEXT("WindLaser"), TEXT("StoneWave"), TEXT("WindTonado"), TEXT("StoneThrow") };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	UAnimMontage* HitAttackMontage;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackEnded);
    UPROPERTY(BlueprintAssignable, Category = "Combat")
    FOnAttackEnded OnAttackEnded;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStunEnded);
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnStunEnded OnStunEnded;

	FVector m_skill_location;

	bool m_is_rotating = false;

	void set_skill_location(FVector skill_location) { m_skill_location = skill_location; }

	void rotate_to_target(float DeltaTime);

	virtual void start_attack(AttackType attack_type);
	virtual void start_attack(AttackType attack_type, FVector attack_location);

	void Attack(AttackType attack_type);
	
	void PlayHitAttackMontage();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	APlayerCharacter* CachedPlayerCharacter = nullptr;

	void FindPlayerCharacter();

public:
	UPROPERTY()
	bool bIsPlayingMontageSection = false;

	UPROPERTY()
	TMap<FName, UCapsuleComponent*> MontageToHitCapsuleMap;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnStunMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	virtual FVector GetFireLocation() override;
	virtual FVector GetCurrentImpactPoint() override;
	virtual FRotator GetCurrentImpactRot() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UNiagaraSystem* WeakPointEffect;

	UPROPERTY()
	UNiagaraComponent* ActiveWeakPointEffect;

	void SpawnWeakPointEffectForCurrentSection(FName SectionName);
	void RemoveWeakPointEffect();

public:
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

	UPROPERTY()
	AMyWindLaser* CurrentWindLaser = nullptr;

	UFUNCTION(BlueprintCallable, Category = "Skill")
	TArray<FVector> GenerateWindTonadoLocations(int32 Count, float MinRadius, float MaxRadius, float MinDistance);

	TSubclassOf<AActor> GetStoneWaveClass() const { return StoneWaveClass; }
	TSubclassOf<AActor> GetStoneSkillClass() const { return StoneSkillClass; }
	TSubclassOf<AActor> GetWindCutterClass() const { return WindCutterClass; }
	TSubclassOf<AActor> GetWindSkillClass() const { return WindSkillClass; }
	TSubclassOf<AActor> GetWindLaserClass() const { return WindLaserClass; }

public:
	// Die
	virtual void Die() override;
	virtual void Reset() override;
	virtual void Respawn() override;
	virtual void Respawn(FVector respawn_location) override;

public:
	// Collision
	UFUNCTION()
	void OnHitCollisionOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Collision
	virtual void Overlap(char skill_type, FVector skill_location) override;

	void PlayStunMontage();

private:
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

	UPROPERTY(EditAnywhere, Category = "Slice")
	FName ProceduralMeshAttachSocketName = TEXT("SliceSocket_Upper");

	UPROPERTY(EditAnywhere, Category = "Slice")
	FName OtherHalfMeshAttachSocketName = TEXT("SliceSocket_Lower");

	UPROPERTY(EditAnywhere)
	float CreateProceduralMeshDistance = 250.0f;

	UPROPERTY()
	UProceduralMeshComponent* CachedOtherHalfMesh = nullptr;

	UPROPERTY()
	UMaterialInterface* CapMaterial;

	int32 NumVertices;

public:
    void CopySkeletalMeshToProcedural(int32 LODIndex);
    void SliceMeshAtBone(FVector SliceNormal, bool bCreateOtherHalf);

	FName GetBoneName() const;
	FName GetSecondBoneName() const;

	UPROPERTY(EditAnywhere, Category = "MySettings")
    class UWidgetComponent* hpFloatingWidget;

    class UMonsterHPBarWidget* MonsterHpBarWidget;

	UFUNCTION(BlueprintCallable, Category = "Hud")
    void ShowHud(float Damage, EClassType Type);
	
	virtual void ReceiveSkillHit(const FSkillInfo& Info, AActor* Causer) override;
public:
    // Hud
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hud")
    TSubclassOf<class ADamagePopupActor> DamagePopupActorClass;
};
