#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "NiagaraSystem.h" 
#include "MyChestActor.generated.h"


// 전방선언 (포인터만 사용 시)
class USceneComponent;
class UBoxComponent;
class USkeletalMeshComponent;
class UAnimationAsset;
class UUserWidget;
class UNiagaraComponent;
class UNiagaraSystem;
class AMyItemDropActor;

UCLASS()
class STATISTIC_API AMyChestActor : public AActor, public IInteractableInterface
{
    GENERATED_BODY()
	
public:	
    AMyChestActor();

protected:
    virtual void BeginPlay() override;

public:	
    virtual void Tick(float DeltaTime) override;

    // 루트
    UPROPERTY(VisibleAnywhere)
    USceneComponent* SceneRoot;

    // 박스 콜리전
    UPROPERTY(VisibleAnywhere)
    UBoxComponent* ChestCollision;

    // 스켈레탈 메시
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* ChestMesh;

    // 열기 애니메이션 (시퀀스)
    UPROPERTY(EditAnywhere)
    UAnimationAsset* OpenAnimSequence;

    // 인터랙트 함수 구현
    virtual void Interact(class APlayerCharacter* InteractingPlayer) override;

protected:
    UFUNCTION()
    void OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<UUserWidget> interactionWidgetClass;

    UUserWidget* interactionWidgetInstance = nullptr;

protected:
    // 나이아가라 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UNiagaraComponent* OpenEffectComponent;

    // (선택) 기본 NiagaraSystem 지정용 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* OpenEffectSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UNiagaraComponent* ItemEffectComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* ItemEffectSystem;

    FTimerHandle ItemEffectTimerHandle;

    UFUNCTION()
    void PlayItemEffect();

    UFUNCTION()
    void OnItemEffectFinished(UNiagaraComponent* PSystem);
    
protected:
    // 드랍된 아이템 액터를 저장할 변수
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class AMyItemDropActor> DroppedItemActorClass;

    bool bIsShrinking = false;
    float ShrinkElapsed = 0.f;
    float ShrinkDuration = 1.0f; // 1초 동안 축소
};
