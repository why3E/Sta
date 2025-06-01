// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MidBossEnemyCharacter.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_MidBossAttack.generated.h"

/**
 * 
 */
UCLASS()
class STATISTIC_API UBTTask_MidBossAttack : public UBTTask_BlueprintBase {
	GENERATED_BODY()
	
public:
	UBTTask_MidBossAttack();

private:
	AMidBossEnemyCharacter* MidBossEnemyCharacter;
	UBehaviorTreeComponent* CachedOwnerComp;

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UFUNCTION()
	void OnAttackEnded();
};
