// MenuGameMode.cpp - Main Menu Implementation
#include "MenuGameMode.h"
#include "Blueprint/UserWidget.h"

AMenuGameMode::AMenuGameMode()
{
    DefaultPawnClass = nullptr; // No pawn needed in menu
}

void AMenuGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (MainMenuWidgetClass)
    {
        MainMenuWidget = CreateWidget<UUserWidget>(GetWorld(), MainMenuWidgetClass);
        if (MainMenuWidget)
        {
            MainMenuWidget->AddToViewport();
            
            // Show mouse cursor
            APlayerController* PC = GetWorld()->GetFirstPlayerController();
            if (PC)
            {
                PC->bShowMouseCursor = true;
                PC->SetInputMode(FInputModeUIOnly());
            }
        }
    }
}
