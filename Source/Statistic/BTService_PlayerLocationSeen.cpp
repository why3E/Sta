// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_PlayerLocationSeen.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"

UBTService_PlayerLocationSeen::UBTService_PlayerLocationSeen()
{
    NodeName = "Update Player Location If Seen";
}
void UBTService_PlayerLocationSeen::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !OwnerComp.GetAIOwner()) return;

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp) return;

    if (OwnerComp.GetAIOwner()->LineOfSightTo(PlayerPawn))
    {
        FVector CurrentLocation = BlackboardComp->GetValueAsVector(GetSelectedBlackboardKey());
        FVector PlayerLocation = PlayerPawn->GetActorLocation();

        // ✅ 100cm 이상 변화가 있을 때만 갱신
        if (FVector::DistSquared(CurrentLocation, PlayerLocation) > FMath::Square(100.f))
        {
            BlackboardComp->SetValueAsVector(GetSelectedBlackboardKey(), PlayerLocation);
        }
    }
    else
    {
        BlackboardComp->ClearValue(GetSelectedBlackboardKey());
    }
}
