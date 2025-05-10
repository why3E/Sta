// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MySkillBase.h"
#include "MixTonadoInterface.h"
#include "Enums.h"
#include "MyMixWindTonado.generated.h"

UCLASS()
class STATISTIC_API AMyMixWindTonado : public AMySkillBase , public IMixTonadoInterface
{
	GENERATED_BODY()
	
public:
	AMyMixWindTonado();

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 토네이도 생성 함수
	void SpawnMixWindTondado(FVector ImpactPoint);

protected:
	// 메시 기반 콜리전 (Convex 포함)
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<class UStaticMeshComponent> CollisionMesh;

	// 나이아가라 파티클 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraComponent> MixWindTonadoNiagaraComponent;

	// 나이아가라 시스템 에셋
	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraSystem> MixWindTonadoEffect;

	// 토네이도 지속 시간
	UPROPERTY(EditAnywhere, Category = "Settings")
	float WindTonadoDuration = 10.0f;

	// 오버랩 이벤트 함수
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	// 필요 시 확장용 가상 함수
	virtual void Overlap();

	// Tick 기반 오버랩 감지
	void CheckOverlappingActors();

	// 타이머
	FTimerHandle CheckOverlapTimerHandle;

	// 종료 시 호출
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float SkillDamage = 20.0f;

	// 속성 타입 (열거형)
	UPROPERTY(EditAnywhere, Category = "Damage")
	EClassType SkillElement = EClassType::CT_Wind;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<UNiagaraSystem> FireEffect;
public:
	// MixWindTonado 스킬 사용
	virtual void SkillMixWindTonado(EClassType MixType) override;
};
