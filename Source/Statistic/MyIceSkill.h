// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MySkillBase.h"
#include "NiagaraComponent.h"
#include "MyIceSkill.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API AMyIceSkill : public AMySkillBase
{
	GENERATED_BODY()

public:
	AMyIceSkill();

	// 얼음 스킬 생성 함수
	void SpawnIceSkill(FVector Location, FRotator Rotation);

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	bool bIsBroken = false;

public:
	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void Overlap(char skill_type) override;
	virtual void Overlap(unsigned short object_id, bool collision_start) override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<class UStaticMeshComponent> CollisionMesh;

	// 나이아가라 파티클 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraComponent> IceWallNiagaraComponent;

	// 나이아가라 에셋
	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraSystem> IceWallEffect;

	// 나이아가라 에셋
	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraSystem> IceWallBreakEffect;

protected:
    UFUNCTION()
    void OnIceWallEffectFinished(UNiagaraComponent* PSystem);

public:
    // 얼음 벽 파괴 및 브레이크 이펙트 재생 후 삭제
    void BreakAndDestroy();
	void SmallAndDestroy();

protected:
    FTimerHandle BreakTimerHandle;

protected:
    int32 SmallCallCount = 0;
    FTimerHandle ShrinkTimerHandle;
    float TargetScale = 0.1f;
    float ShrinkInterpSpeed = 2.0f;

    void ShrinkTick();
};
