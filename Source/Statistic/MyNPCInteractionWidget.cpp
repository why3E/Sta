#include "MyNPCInteractionWidget.h"

void UMyNPCInteractionWidget::SetInteractionText(const FText& NewText)
{
    if (InteractionTextBlock)
    {
        InteractionTextBlock->SetText(NewText);
    }
}
void UMyNPCInteractionWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Button_Out)
    {
        Button_Out->OnClicked.AddDynamic(this, &UMyNPCInteractionWidget::OutButtonClick);
    }
}

void UMyNPCInteractionWidget::OutButtonClick()
{
    OutButtonClickEvent.Broadcast();
}