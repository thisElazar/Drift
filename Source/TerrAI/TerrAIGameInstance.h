// TerrAIGameInstance.h - Game Settings Management
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "TerrAIGameInstance.generated.h"

UENUM(BlueprintType)
enum class EWorldSize : uint8
{
    Small   UMETA(DisplayName = "Small (256x256)"),
    Medium  UMETA(DisplayName = "Medium (513x513)"),
    Large   UMETA(DisplayName = "Large (1024x1024)")
};

UENUM(BlueprintType)
enum class EDefaultTexture : uint8
{
    Wireframe    UMETA(DisplayName = "Digital Wireframe"),
    Natural      UMETA(DisplayName = "Natural Landscape"),
    Hybrid       UMETA(DisplayName = "Hybrid Mode")
};

UCLASS(BlueprintType)
class TERRAI_API UTerrAIGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UTerrAIGameInstance();

    // Game Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    EWorldSize WorldSize = EWorldSize::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    EDefaultTexture DefaultTexture = EDefaultTexture::Natural;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    bool bEnableWaterPhysics = true;

    // Helper functions
    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    int32 GetTerrainSize() const;

    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    void StartNewWorld();

    UFUNCTION(BlueprintCallable, Category = "Game Settings")
    void QuitGame();
};
