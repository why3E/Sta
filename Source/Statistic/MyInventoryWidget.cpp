#include "MyInventoryWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Enums.h"

void UMyInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_HPBig)    Button_HPBig->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnHPBigClicked);
	if (Button_HPSmall)  Button_HPSmall->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnHPSmallClicked);
	if (Button_MPBig)    Button_MPBig->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnMPBigClicked);
	if (Button_MPSmall)  Button_MPSmall->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnMPSmallClicked);
	if (Button_STBig)    Button_STBig->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnSTBigClicked);
	if (Button_STSmall)  Button_STSmall->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnSTSmallClicked);
	if (Button_Make)     Button_Make->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnMakeButtonClicked);
	if (OutButton)		 OutButton->OnClicked.AddDynamic(this, &UMyInventoryWidget::OnOutButtonClicked);
}

void UMyInventoryWidget::SetNeedFlowerInfo(UTexture2D* FlowerTexture, EItemWorldType FlowerType, int32 DivideAmount, UTexture2D* ResultTexture, const FString& ResultText)
{
	// 현재 선택 상태 저장
	CurrentFlowerType = FlowerType;
	CurrentFlowerDivideAmount = DivideAmount;

	// ResultItem 설정에 따라 아이템 타입 갱신
	if      (ResultTexture == Texture_HPBig)    CurrentResultItem = EItemDropType::HealPotion_L;
	else if (ResultTexture == Texture_HPSmall)  CurrentResultItem = EItemDropType::HealPotion_S;
	else if (ResultTexture == Texture_MPBig)    CurrentResultItem = EItemDropType::ManaPotion_L;
	else if (ResultTexture == Texture_MPSmall)  CurrentResultItem = EItemDropType::ManaPotion_S;
	else if (ResultTexture == Texture_STBig)    CurrentResultItem = EItemDropType::StaminaPotion_L;
	else if (ResultTexture == Texture_STSmall)  CurrentResultItem = EItemDropType::StaminaPotion_S;

	// 이미지 설정
	if (NeedItemFlower_Image && FlowerTexture)
		NeedItemFlower_Image->SetBrushFromTexture(FlowerTexture);

	// 텍스트 설정 (보유 개수 / 필요 개수)
	int32 OwnedFlower = InventoryData.GetCount(FlowerType);
	int32 RequiredFlower = DivideAmount;
	if (NeedItem_Flower)
		NeedItem_Flower->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), OwnedFlower, RequiredFlower)));

	int32 OwnedBottle = InventoryData.GetCount(EItemDropType::Bottle);
	int32 RequiredBottle = 1;
	if (NeedItem_Bottle)
		NeedItem_Bottle->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), OwnedBottle, RequiredBottle)));

	if (ResultItem_Text)
		ResultItem_Text->SetText(FText::FromString(ResultText));

	if (Result_Item && ResultTexture)
		Result_Item->SetBrushFromTexture(ResultTexture);
}



void UMyInventoryWidget::OnHPBigClicked()
{
	UE_LOG(LogTemp, Log, TEXT("대형 HP 회복 버튼 클릭"));
	SetNeedFlowerInfo(Texture_HPFlower, EItemWorldType::HP_Flower, 4, Texture_HPBig, TEXT("대형 체력 회복 물약"));
}

void UMyInventoryWidget::OnHPSmallClicked()
{
	UE_LOG(LogTemp, Log, TEXT("소형 HP 회복 버튼 클릭"));
	SetNeedFlowerInfo(Texture_HPFlower, EItemWorldType::HP_Flower, 2, Texture_HPSmall, TEXT("소형 체력 회복 물약"));
}

void UMyInventoryWidget::OnMPBigClicked()
{
	UE_LOG(LogTemp, Log, TEXT("대형 MP 회복 버튼 클릭"));
	SetNeedFlowerInfo(Texture_MPFlower, EItemWorldType::MP_Flower, 4, Texture_MPBig, TEXT("대형 마나 회복 물약"));
}

void UMyInventoryWidget::OnMPSmallClicked()
{
	UE_LOG(LogTemp, Log, TEXT("소형 MP 회복 버튼 클릭"));
	SetNeedFlowerInfo(Texture_MPFlower, EItemWorldType::MP_Flower, 2, Texture_MPSmall, TEXT("소형 마나 회복 물약"));
}

void UMyInventoryWidget::OnSTBigClicked()
{
	UE_LOG(LogTemp, Log, TEXT("대형 ST 회복 버튼 클릭"));
	SetNeedFlowerInfo(Texture_STFlower, EItemWorldType::Stamina_Flower, 4, Texture_STBig, TEXT("대형 스태미나 회복 물약"));
}

void UMyInventoryWidget::OnSTSmallClicked()
{
	UE_LOG(LogTemp, Log, TEXT("소형 ST 회복 버튼 클릭"));
	SetNeedFlowerInfo(Texture_STFlower, EItemWorldType::Stamina_Flower, 2, Texture_STSmall, TEXT("소형 스태미나 회복 물약"));
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
		STBigCount->SetText(FText::FromString(FString::Printf(TEXT("대형 스태미나 회복 물약 %d개"), Count)));
	}

	if (STSmallCount)
	{
		int32 Count = InventoryData.GetCount(EItemDropType::StaminaPotion_S);
		STSmallCount->SetText(FText::FromString(FString::Printf(TEXT("소형 스태미나 회복 물약 %d개"), Count)));
	}
}

void UMyInventoryWidget::UpdateInventory()
{
	if (InventoryData.DropItemCounts.Num() > 0)
	{
		for (const auto& Item : InventoryData.DropItemCounts)
		{
			UE_LOG(LogTemp, Log, TEXT("아이템 종류: %s, 개수: %d"), *UEnum::GetValueAsString(Item.Key), Item.Value);
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("인벤토리에 아이템이 없습니다."));
	}
}

void UMyInventoryWidget::OnMakeButtonClicked()
{
	int32 FlowerCount = InventoryData.GetCount(CurrentFlowerType);
	int32 NeededFlower = FlowerCount / CurrentFlowerDivideAmount;

	int32 BottleCount = InventoryData.GetCount(EItemDropType::Bottle);
	int32 NeededBottle = 1;

	// 실제로 필요한 수량 계산
	int32 RequiredFlower = NeededFlower;
	int32 RequiredBottle = NeededBottle;

	// 요구 수량이 충분한지 확인
	if (InventoryData.GetCount(CurrentFlowerType) >= RequiredFlower &&
		InventoryData.GetCount(EItemDropType::Bottle) >= RequiredBottle)
	{
		// 재료 차감
		InventoryData.RemoveItem(CurrentFlowerType, RequiredFlower);
		InventoryData.RemoveItem(EItemDropType::Bottle, RequiredBottle);

		// 결과 아이템 추가
		InventoryData.AddItem(CurrentResultItem, 1);

		UE_LOG(LogTemp, Log, TEXT("제작 성공! %s 1개 생성됨."), *UEnum::GetValueAsString(CurrentResultItem));

		// UI 갱신
		SetInventoryData(InventoryData);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("재료 부족으로 제작 실패."));
	}
}

void UMyInventoryWidget::OnOutButtonClicked()
{
	RemoveFromParent();
	OnInventoryClosed.Broadcast(); // 델리게이트 호출
}
