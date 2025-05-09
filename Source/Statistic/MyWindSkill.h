// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "MySkillBase.h"
#include "MyWindSkill.generated.h"

UCLASS()
class STATISTIC_API AMyWindSkill : public AMySkillBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMyWindSkill();

	protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;
    // 불벽 생성 함수
    void SpawnWindTonado(FVector Location);

protected:
    // 콜리전 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    TObjectPtr<class UBoxComponent> CollisionComponent;

    // 나이아가라 파티클 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Effects")
    TObjectPtr<UNiagaraComponent> WindTonadoNiagaraComponent;

    // 불벽의 지속 시간
    UPROPERTY(EditAnywhere, Category = "Settings")
    float WindTonadoDuration = 10.0f;

    // 불벽의 나이아가라 시스템 에셋
    UPROPERTY(EditAnywhere, Category = "Effects")
    TObjectPtr<UNiagaraSystem> WindTonadoEffect;

    // 데미지 값
    UPROPERTY(EditAnywhere, Category = "Settings")
    float Damage = 10.0f; 
private:
    // 충돌 처리 함수
    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    virtual void Overlap();

private:
    FTimerHandle CheckOverlapTimerHandle; // 타이머 핸들

    void CheckOverlappingActors(); // 충돌 중인 액터 확인 함수
    void EndPlay(EEndPlayReason::Type EndPlayReason) override; // 종료 시 호출되는 함수
};
