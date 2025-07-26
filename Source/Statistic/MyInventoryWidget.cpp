#include "MyInventoryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Enums.h"

void UMyInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_HPBig)
		Button_HPBig->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnHPBigClicked);

	if (Button_HPSmall)
		Button_HPSmall->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnHPSmallClicked);

	if (Button_MPBig)
		Button_MPBig->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnMPBigClicked);

	if (Button_MPSmall)
		Button_MPSmall->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnMPSmallClicked);

	if (Button_STBig)
		Button_STBig->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnSTBigClicked);

	if (Button_STSmall)
		Button_STSmall->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnSTSmallClicked);

	if (Button_Make)
		Button_Make->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnMakeButtonClicked);
}

void UMyInventoryWidget::OnHPBigClicked()
{
	UE_LOG(LogTemp, Log, TEXT("대형 HP 회복 버튼 클릭"));
}

void UMyInventoryWidget::OnHPSmallClicked()
{
	UE_LOG(LogTemp, Log, TEXT("소형 HP 회복 버튼 클릭"));
}

void UMyInventoryWidget::OnMPBigClicked()
{
	UE_LOG(LogTemp, Log, TEXT("대형 MP 회복 버튼 클릭"));
}

void UMyInventoryWidget::OnMPSmallClicked()
{
	UE_LOG(LogTemp, Log, TEXT("소형 MP 회복 버튼 클릭"));
}

void UMyInventoryWidget::OnSTBigClicked()
{
	UE_LOG(LogTemp, Log, TEXT("대형 ST 회복 버튼 클릭"));
}

void UMyInventoryWidget::OnSTSmallClicked()
{
	UE_LOG(LogTemp, Log, TEXT("소형 ST 회복 버튼 클릭"));
}

void UMyInventoryWidget::OnMakeButtonClicked()
{
	UE_LOG(LogTemp, Log, TEXT("제작 버튼 클릭"));
}

void UMyInventoryWidget::SetInventoryData(const FItemInventory& NewInventory)
{
	InventoryData = NewInventory;

	if (HPBigCount)
	{
		int32 Count = InventoryData.GetCount(EItemDropType::HealPotion_L);
		HPBigCount->SetText(FText::FromString(FString::Printf(TEXT("대형 체력 회복 물약 %d개"), Count)));
	}

	if (HPSmallCount)
	{
		int32 Count = InventoryData.GetCount(EItemDropType::HealPotion_S);
		HPSmallCount->SetText(FText::FromString(FString::Printf(TEXT("소형 체력 회복 물약 %d개"), Count)));
	}

	if (MPBigCount)
	{
		int32 Count = InventoryData.GetCount(EItemDropType::ManaPotion_L);
		MPBigCount->SetText(FText::FromString(FString::Printf(TEXT("대형 마나 회복 물약 %d개"), Count)));
	}

	if (MPSmallCount)
	{
		int32 Count = InventoryData.GetCount(EItemDropType::ManaPotion_S);
		MPSmallCount->SetText(FText::FromString(FString::Printf(TEXT("소형 마나 회복 물약 %d개"), Count)));
	}

	if (STBigCount)
	{
		int32 Count = InventoryData.GetCount(EItemDropType::StaminaPotion_L);
		STBigCount->SetText(FText::FromString(FString::Printf(TEXT("대형 스테미나 회복 물약 %d개"), Count)));
	}

	if (STSmallCount)
	{
		int32 Count = InventoryData.GetCount(EItemDropType::StaminaPotion_S);
		STSmallCount->SetText(FText::FromString(FString::Printf(TEXT("소형 스테미나 회복 물약 %d개"), Count)));
	}
}