#include "MyLobbyWidget.h"
#include "Components/Button.h"
#include "Components/EditableText.h"

FString UMyLobbyWidget::GetEnteredIP() const
{
    if (IPText) {
        return IPText->GetText().ToString();  // EditableText도 GetText() 있음
    }

    return FString();
}

void UMyLobbyWidget::HandleStartClicked()
{
    FString IP = GetEnteredIP();

    OnStartPressed.Broadcast(IP);
}

bool UMyLobbyWidget::Initialize()
{
    if (!Super::Initialize()) return false;

    if (StartButton)
    {
        StartButton->OnClicked.AddDynamic(this, &UMyLobbyWidget::HandleStartClicked);
    }

    return true;
}
