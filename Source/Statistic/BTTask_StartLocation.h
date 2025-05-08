#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_StartLocation.generated.h"

UCLASS()
class STATISTIC_API UBTTask_StartLocation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_StartLocation();

protected:
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override; // TickTask 추가
};