#include "MyInteractionUI.h"

void UMyInteractionUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartAnim)
	{
		PlayAnimation(StartAnim);
	}
}

void UMyInteractionUI::PlayStartAnimation()
{
	if (StartAnim)
	{
		PlayAnimation(StartAnim, 0.f, 1); // 시작 시간 0, 반복 1회
	}
}