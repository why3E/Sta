#pragma once

#include "SESSION.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableInterface.h"
#include "MyMagicStatue.generated.h"

class UBoxComponent;
class USceneComponent;
class APlayerCharacter;
class UMyFadeWidget;
class UMyUserSelectorUI;
class UUserWidget;
class APlayerController;

UCLASS()
class STATISTIC_API AMyMagicStatue : public AActor, public IInteractableInterface
{
    GENERATED_BODY()
    
public:    
    AMyMagicStatue();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, Category="Components")
    UBoxComponent* boxCollision;

    UPROPERTY(EditAnywhere, Category="Components")
    USceneComponent* sceneComp;

    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UUserWidget> interactionWidgetClass;

    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UMyUserSelectorUI> selectorWidget;

    UPROPERTY(EditAnywhere, Category="UI")
    TSubclassOf<UMyFadeWidget> fadeWidget;

    UPROPERTY(EditAnywhere, Category="Teleport")
    int32 StatueNumber = STATUE_ID_START;

protected:
    UFUNCTION()
    void OnBeginOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnEndOverlapCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UFUNCTION()
    void OnSelectorClosed();
    
    UFUNCTION()
    void OnSelectorMove();

public:
    virtual void Interact(APlayerCharacter* InteractingPlayer) override;
    void StartTeleportWithFade();

protected:
    UFUNCTION()
    void HandleFadeInFinished();

    UFUNCTION()
    void RemoveFadeWidget();

    void PerformTeleport();

private:
    APlayerCharacter* cachedPlayer = nullptr;
    APlayerController* cachedController = nullptr;

    UUserWidget* interactionWidgetInstance = nullptr;
    UMyUserSelectorUI* selectorWidgetInstance = nullptr;
    UMyFadeWidget* fadeWidgetInstance = nullptr;
	
    // 멤버 변수로 다음 석상과 첫 번째 석상 저장
    AMyMagicStatue* NextStatue = nullptr;
    AMyMagicStatue* FirstStatue = nullptr;
};