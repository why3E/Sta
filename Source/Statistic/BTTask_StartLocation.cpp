#include "BTTask_StartLocation.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_StartLocation::UBTTask_StartLocation()
{
    NodeName = TEXT("Move To Start Location Smooth");
    bNotifyTick = true; // TickTask를 활성화
}

EBTNodeResult::Type UBTTask_StartLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::InProgress; // TickTask에서 이동을 처리
}

void UBTTask_StartLocation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController) return;

    ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
    if (!Character) return;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return;

    // 블랙보드에서 StartLocation 값을 가져옴
    FVector StartLocation = BlackboardComp->GetValueAsVector(TEXT("StartLocation"));
    FVector MyLocation = Character->GetActorLocation();

    // StartLocation으로의 방향 계산
    FVector ToTarget = (StartLocation - MyLocation);
    FVector Direction = ToTarget.GetSafeNormal2D();

    // 방향이 유효하면 회전 및 이동 적용
    if (!Direction.IsNearlyZero())
    {
        FRotator TargetRot = Direction.Rotation();
        FRotator NewRot = FMath::RInterpTo(Character->GetActorRotation(), TargetRot, DeltaSeconds, 3.0f);
        Character->SetActorRotation(NewRot);

        // 이동 적용
        Character->AddMovementInput(Direction, 1.0f);
    }

    // 도착했는지 확인
    if (ToTarget.Size2D() < 100.f) // 원하는 도착 거리
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}