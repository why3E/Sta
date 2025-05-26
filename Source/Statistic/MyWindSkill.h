// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "MySkillBase.h"
#include "Enums.h"
#include "MixTonadoInterface.h"
#include "MyWindSkill.generated.h"

UCLASS()
class STATISTIC_API AMyWindSkill : public AMySkillBase, public IMixTonadoInterface
{
	GENERATED_BODY()
	
public:	
	AMyWindSkill();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 토네이도 생성 함수
	void SpawnWindTonado(FVector Location);

protected:
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<class UStaticMeshComponent> CollisionMesh;

	// 나이아가라 파티클 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraComponent> WindTonadoNiagaraComponent;

	// 나이아가라 에셋
	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraSystem> WindTonadoEffect;

	// 지속 시간
	UPROPERTY(EditAnywhere, Category = "Settings")
	float WindTonadoDuration = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<UNiagaraSystem> FireEffect;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<UNiagaraSystem> IceEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
    TSubclassOf<class AMyMixWindTonado> MixWindTonadoClass;

private:
	// 충돌 감지 함수
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// Tick 기반 충돌 확인용 타이머
	FTimerHandle CheckOverlapTimerHandle;

	// 반복적으로 오버랩된 액터를 감지
	void CheckOverlappingActors();

	// 스폰된 액터 제거 시 호출
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Overlap(AActor* OtherActor);
	virtual void Overlap(ACharacter* OtherActor);

	// MixWindTonado 스킬 사용
	virtual void SkillMixWindTonado(EClassType MixType, unsigned short skill_id) override;
	void SpawnMixTonado(unsigned short skill_id);
};
