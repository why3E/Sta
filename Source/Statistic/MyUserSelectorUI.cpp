#include "MyUserSelectorUI.h"
#include "Components/Button.h"
#include "MyMagicStatue.h"

void UMyUserSelectorUI::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Yes)
    {
        Button_Yes->OnClicked.AddDynamic(this, &UMyUserSelectorUI::YesButtonClick);
    }

    if (Button_No)
    {
        Button_No->OnClicked.AddDynamic(this, &UMyUserSelectorUI::NoButtonClick);
    }
}

void UMyUserSelectorUI::NativeDestruct()
{
    Super::NativeDestruct();

    // NO 클릭시만 OnSelectorClosed 호출하기 때문에 여기서는 호출하지 않음
}

void UMyUserSelectorUI::YesButtonClick()
{
    if (StatueActor)
    {
        StatueActor->StartTeleportWithFade();
    }

    RemoveFromParent();
}

void UMyUserSelectorUI::NoButtonClick()
{
    RemoveFromParent();

    // NO 눌렀을 때만 이벤트 방송
    OnSelectorClosed.Broadcast();
}
