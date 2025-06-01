// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBaseAttackCheck.h"
#include "AnimationAttackInterface.h"

void UMyBaseAttackCheck::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp)
	{
		// 공격 성공 여부 체크
		IAnimationAttackInterface* AttackPawn = Cast<IAnimationAttackInterface>(MeshComp->GetOwner());

		if (AttackPawn)
		{
			AttackPawn->BaseAttackCheck();
		}
	}
}