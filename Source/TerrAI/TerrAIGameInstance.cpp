// TerrAIGameInstance.cpp - Game Settings Implementation
#include "TerrAIGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MasterController.h"

UTerrAIGameInstance::UTerrAIGameInstance()
{
    WorldSize = EWorldSize::Medium;
    DefaultTexture = EDefaultTexture::Natural;
    bEnableWaterPhysics = true;
    bHasPreviewSettings = false;
}

int32 UTerrAIGameInstance::GetTerrainSize() const
{
    switch (WorldSize)
    {
        case EWorldSize::Small: return 257;
        case EWorldSize::Medium: return 513;
        case EWorldSize::Large: return 1025;
        default: return 513;
    }
}

void UTerrAIGameInstance::SetWorldSize(EWorldSize NewSize)
{
    WorldSize = NewSize;
    UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: World size set to %d"), (int32)NewSize);
    
    // If we're in a game world, update the MasterController immediately
    ApplyWorldSizeToMasterController();
}

void UTerrAIGameInstance::StartNewWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: Attempting to load BaseMap..."));
    UGameplayStatics::OpenLevel(this, FName("BaseMap"));
    UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: OpenLevel called for BaseMap"));
}

void UTerrAIGameInstance::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

// Menu Integration Functions
void UTerrAIGameInstance::SetPreviewSettings(EWorldSize InWorldSize, EDefaultTexture InTexture, bool bInWeather)
{
    WorldSize = InWorldSize;
    DefaultTexture = InTexture;
    bEnableWaterPhysics = bInWeather;
    bHasPreviewSettings = true;
    
    UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: Preview settings stored"));
}

void UTerrAIGameInstance::StartGameWithPreviewSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: Starting game with preview settings"));
    
    // Apply settings before level transition
    if (bHasPreviewSettings)
    {
        ApplyPreviewSettingsToGame();
    }
    
    // Load the main game level
    UGameplayStatics::OpenLevel(this, FName("BaseMap"));
}

void UTerrAIGameInstance::ReturnToMainMenu()
{
    UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: Returning to main menu"));
    UGameplayStatics::OpenLevel(this, FName("MainMenuWorld"));
}

void UTerrAIGameInstance::ApplyPreviewSettingsToGame()
{
    UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: Applying preview settings to game"));
    // Settings already applied in SetPreviewSettings
    ApplyWorldSizeToMasterController();
}

void UTerrAIGameInstance::ApplyWorldSizeToMasterController()
{
    // Find MasterController in the current world and apply settings
    if (UWorld* World = GetWorld())
    {
        if (AActor* MasterControllerActor = UGameplayStatics::GetActorOfClass(World, AMasterWorldController::StaticClass()))
        {
            if (AMasterWorldController* MasterController = Cast<AMasterWorldController>(MasterControllerActor))
            {
                UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: Applying world size %d to MasterController"), (int32)WorldSize);
                MasterController->SetWorldSizeFromUI(WorldSize);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("TerrAIGameInstance: MasterController not found - settings will be applied when it spawns"));
        }
    }
}
