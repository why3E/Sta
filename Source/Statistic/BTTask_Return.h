// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_Return.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API UBTTask_Return : public UBTTask_BlueprintBase {
	GENERATED_BODY()

private:
	FVector StartLocation;
	
public:
	UBTTask_Return();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
