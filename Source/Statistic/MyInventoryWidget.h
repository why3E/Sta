#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enums.h"
#include "MyInventoryWidget.generated.h"

class UTextBlock;
class UButton;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryClosed);

UCLASS()
class STATISTIC_API UMyInventoryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 닫기 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnInventoryClosed OnInventoryClosed;
public:
	// 버튼 클릭 함수들
	UFUNCTION()
	void OnHPBigClicked();

	UFUNCTION()
	void OnHPSmallClicked();

	UFUNCTION()
	void OnMPBigClicked();

	UFUNCTION()
	void OnMPSmallClicked();

	UFUNCTION()
	void OnSTBigClicked();

	UFUNCTION()
	void OnSTSmallClicked();

	UFUNCTION()
	void OnMakeButtonClicked();

	UFUNCTION()
	void OnOutButtonClicked();

protected:
	virtual void NativeConstruct() override;

	// 텍스트 (카운트)
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* STBigCount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* STSmallCount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HPBigCount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HPSmallCount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MPBigCount;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MPSmallCount;

	// 버튼
	UPROPERTY(meta = (BindWidget))
	class UButton* Button_HPBig;

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_HPSmall;

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_MPBig;

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_MPSmall;

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_STBig;

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_STSmall;

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Make;

	UPROPERTY(meta = (BindWidget))
	class UButton* OutButton;

	// 제작 관련 텍스트
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ResultItem_Text;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NeedItem_Flower;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NeedItem_Bottle;

	// 제작 관련 이미지
	UPROPERTY(meta = (BindWidget))
	class UImage* NeedItemFlower_Image;

	UPROPERTY(meta = (BindWidget))
	class UImage* Result_Item;

public:
	UFUNCTION(BlueprintCallable)
	void SetInventoryData(const FItemInventory& NewInventory);

	void UpdateInventory();

protected:
	// 전달된 인벤토리 정보
	UPROPERTY()
	FItemInventory InventoryData;

	void SetNeedFlowerInfo(UTexture2D* FlowerTexture, EItemWorldType FlowerType, int32 DivideAmount, UTexture2D* ResultTexture, const FString& ResultText);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_HPBig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_HPSmall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_MPBig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_MPSmall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_STBig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_STSmall;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_HPFlower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_MPFlower;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory|Textures")
	TObjectPtr<UTexture2D> Texture_STFlower;
protected:
	// 현재 선택된 결과 아이템 정보
	EItemDropType CurrentResultItem = EItemDropType::Bottle;
	EItemWorldType CurrentFlowerType = EItemWorldType::HP_Flower;
	int32 CurrentFlowerDivideAmount = 1;
};
