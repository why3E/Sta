#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "MySkillBase.h"
#include "Enums.h"
#include "MyFireSkill.generated.h"

UCLASS()
class STATISTIC_API AMyFireSkill : public AMySkillBase
{
    GENERATED_BODY()

public:
    AMyFireSkill();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

	virtual void PostInitializeComponents() override;
    
    // 불벽 생성 함수
    void SpawnFireWall(FVector Location, FRotator Rotation);

    virtual void Overlap(char skill_type) override;
    virtual void Overlap(unsigned short object_id, bool collision_start) override;

protected:
    // 콜리전 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    TObjectPtr<class UBoxComponent> CollisionComponent;

    // 나이아가라 파티클 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Effects")
    TObjectPtr<UNiagaraComponent> FireWallNiagaraComponent;

    UPROPERTY(VisibleAnywhere, Category = "Effects")
    TObjectPtr<UNiagaraComponent> FireWallVisualEffectComponent;

    // 추가적인 시각적 효과용 나이아가라 시스템 에셋
    UPROPERTY(EditAnywhere, Category = "Effects")
    TObjectPtr<UNiagaraSystem> FireWallVisualEffectSystem;

    // 불벽의 지속 시간
    UPROPERTY(EditAnywhere, Category = "Settings")
    float FireWallDuration = 10.0f;

    // 불벽의 나이아가라 시스템 에셋
    UPROPERTY(EditAnywhere, Category = "Effects")
    TObjectPtr<UNiagaraSystem> FireWallEffect;

private:
    // 충돌 처리 함수
    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
    FTimerHandle CheckOverlapTimerHandle; // 타이머 핸들

    void CheckOverlappingActors(); // 충돌 중인 액터 확인 함수
    void EndPlay(EEndPlayReason::Type EndPlayReason) override; // 종료 시 호출되는 함수
};