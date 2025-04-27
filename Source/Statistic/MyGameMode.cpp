// MyGameMode.cpp
#include "MyGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

AMyGameMode::AMyGameMode()
{
	// 예: BP_Player를 기본 폰으로 설정
	static ConstructorHelpers::FClassFinder<APawn> PawnClassRef(TEXT("/Game/player_anim/MyPlayerCharacter.MyPlayerCharacter_C"));
    if (PawnClassRef.Succeeded())
    {
        DefaultPawnClass = PawnClassRef.Class;
    }
    
    static ConstructorHelpers::FClassFinder<APlayerController> ControllerClassRef(TEXT("/Script/CoreUObject.Class'/Script/Statistic.MyPlayerController'"));
    if (ControllerClassRef.Succeeded())
    {
        PlayerControllerClass = ControllerClassRef.Class;
    }
}
