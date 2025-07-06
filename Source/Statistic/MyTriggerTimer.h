#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyTriggerTimer.generated.h"

class UTextBlock;

UCLASS()
class STATISTIC_API UMyTriggerTimer : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 남은 시간 표시용 함수
	UFUNCTION(BlueprintCallable)
	void UpdateTime(int32 Seconds);

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TimeText;
};
