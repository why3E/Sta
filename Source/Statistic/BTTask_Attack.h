// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyCharacter.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"


class AEnemyCharacter;
class UBehaviorTreeComponent;

UCLASS()
class STATISTIC_API UBTTask_Attack : public UBTTaskNode {
	GENERATED_BODY()

public:
	UBTTask_Attack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	UFUNCTION()
	void OnAttackEnded();

private:
	AEnemyCharacter* EnemyCharacter;
	UBehaviorTreeComponent* CachedOwnerComp;
};
