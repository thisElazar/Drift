// TerrAIGameInstance.cpp - Game Settings Implementation
#include "TerrAIGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UTerrAIGameInstance::UTerrAIGameInstance()
{
    WorldSize = EWorldSize::Medium;
    DefaultTexture = EDefaultTexture::Natural;
    bEnableWaterPhysics = true;
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

void UTerrAIGameInstance::StartNewWorld()
{
    UGameplayStatics::OpenLevel(this, FName("GameLevel"));
}

void UTerrAIGameInstance::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}
