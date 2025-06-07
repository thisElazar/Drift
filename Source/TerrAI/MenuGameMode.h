// MenuGameMode.h - Main Menu Controller
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Components/Widget.h"
#include "MenuGameMode.generated.h"

class UUserWidget;

UCLASS()
class TERRAI_API AMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMenuGameMode();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> MainMenuWidgetClass;

    UPROPERTY()
    class UUserWidget* MainMenuWidget;
};
