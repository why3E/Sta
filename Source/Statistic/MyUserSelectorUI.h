#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyUserSelectorUI.generated.h"

class UButton;
class AMyMagicStatue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectorClosed);

UCLASS()
class STATISTIC_API UMyUserSelectorUI : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    UButton* Button_Yes;

    UPROPERTY(meta = (BindWidget))
    UButton* Button_No;

    // YES/NO 버튼 클릭 핸들러
    UFUNCTION()
    void YesButtonClick();

    UFUNCTION()
    void NoButtonClick();

    // UI 닫힐 때 방송용 이벤트 (NO 눌렀을 때만)
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSelectorClosed OnSelectorMove;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSelectorClosed OnSelectorClosed;

};
