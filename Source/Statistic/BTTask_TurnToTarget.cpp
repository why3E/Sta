#include "BTTask_TurnToTarget.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTTask_TurnToTarget::UBTTask_TurnToTarget()
{
    NodeName = TEXT("Chase Player Smooth");
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_TurnToTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::InProgress;
}

void UBTTask_TurnToTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return;

    ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
    if (!Character) return;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return;

    // 블랙보드에서 목표 위치와 시작 위치를 가져옴
    FVector TargetLocation = BlackboardComp->GetValueAsVector(TEXT("PlayerLocation"));
    FVector StartLocation = BlackboardComp->GetValueAsVector(TEXT("StartLocation"));
    FVector MyLocation = Character->GetActorLocation();

    // 플레이어와 시작 위치 간의 거리 계산
    float DistanceToStart = FVector::Dist2D(StartLocation, TargetLocation);

    // 거리가 3000 이상이면 태스크 종료
    if (DistanceToStart > 3000.0f)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector ToTarget = (TargetLocation - MyLocation);
    FVector Direction = ToTarget.GetSafeNormal2D();

    if (!Direction.IsNearlyZero())
    {
        FRotator TargetRot = Direction.Rotation();
        FRotator NewRot = FMath::RInterpTo(Character->GetActorRotation(), TargetRot, DeltaSeconds, 3.0f);
        Character->SetActorRotation(NewRot);

        // 이동 적용
        Character->AddMovementInput(Direction, 1.0f);
    }

    // LastKnownPlayerLocation에 현재 TargetLocation 저장
    BlackboardComp->SetValueAsVector(TEXT("LastKnownPlayerLocation"), TargetLocation);

    // 도착했는지 확인
    if (ToTarget.Size2D() < 100.f) // 원하는 도착 거리
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}