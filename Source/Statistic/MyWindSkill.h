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
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

public:
	// 토네이도 스폰
	void SpawnWindTonado(FVector Location);

	// 스킬 믹스 처리
	virtual void SkillMixWindTonado(EClassType MixType, unsigned short skill_id) override;
	void SpawnMixTonado(unsigned short skill_id);

	// 외부에서 수동 충돌 감지할 때 호출
	virtual void Overlap(AActor* OtherActor) override;
	virtual void Overlap(ACharacter* OtherActor) override;

protected:
	/** 콜리전 메시 */
	UPROPERTY(VisibleAnywhere, Category = "Collision")
	TObjectPtr<class UStaticMeshComponent> CollisionMesh;

	/** 나이아가라 토네이도 이펙트 */
	UPROPERTY(VisibleAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraComponent> WindTonadoNiagaraComponent;

	/** 기본 이펙트 */
	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<class UNiagaraSystem> WindTonadoEffect;

	/** 속성에 따른 이펙트 */
	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<UNiagaraSystem> FireEffect;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TObjectPtr<UNiagaraSystem> IceEffect;

	/** 토네이도 지속 시간 */
	UPROPERTY(EditAnywhere, Category = "Settings")
	float WindTonadoDuration = 10.0f;

	/** 믹스 토네이도 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	TSubclassOf<class AMyMixWindTonado> MixWindTonadoClass;

	/** 현재 오버랩된 캐릭터 목록 */
	UPROPERTY()
	TSet<class APlayerCharacter*> OverlappingCharacters;

private:
	bool bIsValid = true;

	/** 컴포넌트 오버랩 진입 시 */
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** 컴포넌트 오버랩 이탈 시 */
	UFUNCTION()
	void OnEndOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	/** Tick 기반 오버랩 검사 (예비용) */
	void CheckOverlappingActors();

	/** 충돌 체크 타이머 */
	FTimerHandle CheckOverlapTimerHandle;
};
