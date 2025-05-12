#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundBase.h" // 배경음악을 위한 헤더 추가
#include "MyMusicTriggerBox.generated.h"

UCLASS()
class STATISTIC_API AMyMusicTriggerBox : public ATriggerBox
{
    GENERATED_BODY()

public:
    AMyMusicTriggerBox();

protected:
    virtual void BeginPlay() override;

    // 트리거 박스에 들어왔을 때 호출되는 함수
    UFUNCTION()
    void OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

    // 트리거 박스를 나갔을 때 호출되는 함수
    UFUNCTION()
    void OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

protected:
    // 배경음악
    UPROPERTY(EditAnywhere, Category = "Sound")
    TObjectPtr<class USoundBase> BackgroundMusic;

    // 배경음악이 재생 중인지 확인
    bool bIsMusicPlaying;
};