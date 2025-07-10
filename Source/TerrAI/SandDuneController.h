/**
 * ============================================
 * TERRAI SAND DUNE CONTROLLER
 * ============================================
 * Purpose: Specialized sand dune formation and movement system
 * Extracted from GeologyController for better modularity
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MasterController.h"
#include "SandDuneController.generated.h"

class ADynamicTerrain;
class UAtmosphericSystem;
class AGeologyController;

UENUM(BlueprintType)
enum class EDuneType : uint8
{
    Barchan        UMETA(DisplayName = "Barchan (Crescent)"),
    Transverse     UMETA(DisplayName = "Transverse (Wave)"),
    Linear         UMETA(DisplayName = "Linear (Seif)"),
    Star           UMETA(DisplayName = "Star"),
    Parabolic      UMETA(DisplayName = "Parabolic")
};

USTRUCT(BlueprintType)
struct FSandDuneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Height = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Width = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Length = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDuneType DuneType = EDuneType::Barchan;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MigrationDirection = FVector(1, 0, 0);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SandVolume = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Stability = 0.5f; // 0 = highly mobile, 1 = stabilized
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VegetationCover = 0.0f; // 0-1, affects stability
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MigrationRate = 5.0f; // meters per year
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float Age = 0.0f; // in geological time units
};

USTRUCT(BlueprintType)
struct FSandTransportData
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Position = FVector::ZeroVector;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float SandFlux = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector TransportDirection = FVector::ZeroVector;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    float SaltationHeight = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TERRAI_API ASandDuneController : public AActor, public IScalableSystem
{
    GENERATED_BODY()

public:
    ASandDuneController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ===== INITIALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Sand Dunes")
    void Initialize(ADynamicTerrain* Terrain, AGeologyController* Geology);
    
    // ===== DUNE PARAMETERS =====
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sand Dunes")
    bool bEnableSandDunes = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sand Dunes")
    TArray<FSandDuneData> SandDunes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float BaseWindStrength = 15.0f; // m/s
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector PrevailingWindDirection = FVector(1, 0, 0);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindVariability = 0.2f; // 0-1, seasonal variation
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transport")
    float SandTransportThreshold = 10.0f; // m/s, minimum wind for transport
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transport")
    float SaltationEfficiency = 0.5f; // 0-1, sand hopping efficiency
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transport")
    float MaxTransportRate = 100.0f; // kg/m/s
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float DuneFormationThreshold = 500.0f; // minimum sand volume
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
    float DuneMergeDistance = 50.0f; // distance for dune merging
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
    float VegetationGrowthRate = 0.01f; // per geological time unit
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
    float VegetationStabilizationFactor = 0.8f; // reduction in mobility

    // ===== CORE FUNCTIONS =====
    
    UFUNCTION(BlueprintCallable, Category = "Sand Dunes")
    void UpdateSandDunes(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Sand Transport")
    void CalculateSandTransport(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Sand Transport")
    float GetSandFluxAtLocation(FVector Location) const;
    
    UFUNCTION(BlueprintCallable, Category = "Dune Formation")
    void FormNewDune(FVector Location, float InitialVolume, EDuneType Type);
    
    UFUNCTION(BlueprintCallable, Category = "Dune Formation")
    void MergeDunes(int32 DuneIndex1, int32 DuneIndex2);
    
    UFUNCTION(BlueprintCallable, Category = "Dune Evolution")
    void EvolveDuneShape(FSandDuneData& Dune, float DeltaTime);
    
    // ===== WIND INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Wind")
    void UpdateWindData();
    
    UFUNCTION(BlueprintPure, Category = "Wind")
    FVector GetEffectiveWindAtLocation(FVector Location) const;
    
    UFUNCTION(BlueprintCallable, Category = "Wind")
    void OnWindChanged(FVector NewWindDirection, float NewWindSpeed);
    
    // ===== TERRAIN INTERACTION =====
    
    UFUNCTION(BlueprintCallable, Category = "Terrain")
    void ApplyDunesToTerrain();
    
    UFUNCTION(BlueprintPure, Category = "Terrain")
    float GetDuneHeightAtLocation(FVector Location) const;
    
    // ===== VISUALIZATION =====
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowSandDunes(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Visualization")
    void ShowSandTransport(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintSandDuneStats() const;
    
    // ===== ISCALABLESYSTEM INTERFACE =====
    
    virtual void ConfigureFromMaster(const FWorldScalingConfig& Config) override;
    virtual void SynchronizeCoordinates(const FWorldCoordinateSystem& Coords) override;
    virtual bool IsSystemScaled() const override;
    
    // ===== MASTER CONTROLLER INTEGRATION =====
    
    UFUNCTION(BlueprintCallable, Category = "World Scaling")
    void RegisterWithMasterController(AMasterWorldController* Master);
    
    UFUNCTION(BlueprintPure, Category = "World Scaling")
    bool IsRegisteredWithMaster() const;
    
    UFUNCTION(BlueprintPure, Category = "World Scaling")
    FString GetScalingDebugInfo() const;

private:
    // ===== SYSTEM REFERENCES =====
    
    UPROPERTY()
    ADynamicTerrain* TargetTerrain = nullptr;
    
    UPROPERTY()
    AGeologyController* GeologyController = nullptr;
    
    UPROPERTY()
    UAtmosphericSystem* CachedAtmosphereSystem = nullptr;
    
    // ===== INTERNAL STATE =====
    
    UPROPERTY()
    TArray<FSandTransportData> TransportGrid;
    
    UPROPERTY()
    FVector CurrentWindVector = FVector::ZeroVector;
    
    UPROPERTY()
    float DuneUpdateTimer = 0.0f;
    
    UPROPERTY()
    bool bSystemInitialized = false;
    
    // ===== WORLD SCALING STATE =====
    
    UPROPERTY()
    AMasterWorldController* MasterController = nullptr;
    
    FWorldScalingConfig CurrentWorldConfig;
    FWorldCoordinateSystem CurrentCoordinateSystem;
    
    UPROPERTY()
    bool bIsScaledByMaster = false;
    
    UPROPERTY()
    bool bIsRegisteredWithMaster = false;
    
    // ===== INTERNAL FUNCTIONS =====
    
    void ProcessSaltation(float DeltaTime);
    void ProcessCreep(float DeltaTime);
    void ProcessAvalanche(FSandDuneData& Dune, float DeltaTime);
    float CalculateAngleOfRepose(const FSandDuneData& Dune) const;
    void StabilizeDune(FSandDuneData& Dune, float DeltaTime);
    void GenerateTransportGrid();
};