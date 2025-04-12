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

	// 블랙보드에서 목표 위치를 가져옴
	FVector TargetLocation = BlackboardComp->GetValueAsVector(TEXT("PlayerLocation"));
	FVector MyLocation = Character->GetActorLocation();

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

	// 도착했는지 확인
	if (ToTarget.Size2D() < 100.f) // 원하는 도착 거리
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}