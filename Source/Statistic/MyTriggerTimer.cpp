#include "MyTriggerTimer.h"
#include "Components/TextBlock.h"

void UMyTriggerTimer::UpdateTime(int32 Seconds)
{
    FString DisplayText = FString::Printf(TEXT("%02d"), Seconds);
    if (TimeText)
    {
        TimeText->SetText(FText::FromString(DisplayText));
    }
}