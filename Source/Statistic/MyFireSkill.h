#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "MyFireSkill.generated.h"

UCLASS()
class STATISTIC_API AMyFireSkill : public AActor
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

protected:
    // 콜리전 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Collision")
    TObjectPtr<class UBoxComponent> CollisionComponent;

    // 나이아가라 파티클 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Effects")
    TObjectPtr<UNiagaraComponent> FireWallNiagaraComponent;

    // 불벽의 지속 시간
    UPROPERTY(EditAnywhere, Category = "Settings")
    float FireWallDuration = 10.0f;

    // 불벽의 나이아가라 시스템 에셋
    UPROPERTY(EditAnywhere, Category = "Effects")
    TObjectPtr<UNiagaraSystem> FireWallEffect;

    // 데미지 값
    UPROPERTY(EditAnywhere, Category = "Settings")
    float Damage = 10.0f;
private:
    // 충돌 처리 함수
    UFUNCTION()
    void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};