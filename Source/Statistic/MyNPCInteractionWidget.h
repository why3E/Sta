#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "MyNPCInteractionWidget.generated.h"

// 델리게이트 타입 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOutButtonClick);

UCLASS()
class STATISTIC_API UMyNPCInteractionWidget : public UUserWidget
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintCallable)
    void SetInteractionText(const FText& NewText);

    UFUNCTION()
    void OutButtonClick();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOutButtonClick OutButtonClickEvent;

    virtual void NativeConstruct() override;

protected:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* InteractionTextBlock;

    UPROPERTY(meta = (BindWidget))
    UButton* Button_Out;
};