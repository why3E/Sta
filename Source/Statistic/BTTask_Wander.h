// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Wander.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API UBTTask_Wander : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_Wander();
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "AI")
    float SearchRadius = 1000.0f;  
};
