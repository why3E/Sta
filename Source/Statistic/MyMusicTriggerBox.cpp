#include "MyMusicTriggerBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

AMyMusicTriggerBox::AMyMusicTriggerBox()
{
    // 배경음악 재생 상태 초기화
    bIsMusicPlaying = false;
}

void AMyMusicTriggerBox::BeginPlay()
{
    Super::BeginPlay();

    // 트리거 박스의 오버랩 이벤트 바인딩
    OnActorBeginOverlap.AddDynamic(this, &AMyMusicTriggerBox::OnOverlapBegin);
    OnActorEndOverlap.AddDynamic(this, &AMyMusicTriggerBox::OnOverlapEnd);
}

void AMyMusicTriggerBox::OnOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
    // 플레이어가 트리거 박스에 들어왔을 때 배경음악 재생
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()) && !bIsMusicPlaying)
    {
        if (BackgroundMusic)
        {
            // 배경음악을 플레이어의 RootComponent에 연결
            UGameplayStatics::SpawnSoundAttached(BackgroundMusic, OtherActor->GetRootComponent());
            bIsMusicPlaying = true;
        }
    }
}

void AMyMusicTriggerBox::OnOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
    // 플레이어가 트리거 박스를 나갔을 때 배경음악 정지
    if (OtherActor && OtherActor->IsA(ACharacter::StaticClass()) && bIsMusicPlaying)
    {
        // 배경음악 정지 로직 추가 가능 (필요 시)
        bIsMusicPlaying = false;
    }
}