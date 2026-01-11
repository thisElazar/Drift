// DEMImporter.h
// Digital Elevation Model Import System - UE5 Native Implementation
// Supports: SRTM HGT, ESRI ASCII Grid, PNG16, TIFF (with world files), RAW
// Zero external dependencies - uses only UE5 built-in modules

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DEMImporter.generated.h"

// Forward declaration for MasterController (avoid circular dependency)
class AMasterWorldController;

/**
 * Supported DEM file formats (all UE5-native)
 */
UENUM(BlueprintType)
enum class EDEMFormat : uint8
{
    SRTM_HGT    UMETA(DisplayName = "SRTM Height (.hgt)"),
    ASCIIGrid   UMETA(DisplayName = "ESRI ASCII Grid (.asc)"),
    PNG16       UMETA(DisplayName = "16-bit PNG (.png)"),
    TIFF        UMETA(DisplayName = "TIFF (.tif, .tiff)"),
    RAW         UMETA(DisplayName = "Raw Binary (.raw)"),
    Auto        UMETA(DisplayName = "Auto-detect from extension")
};

/**
 * Resampling methods for DEM scaling
 */
UENUM(BlueprintType)
enum class EDEMResampleMethod : uint8
{
    NearestNeighbor    UMETA(DisplayName = "Nearest Neighbor (Fast)"),
    Bilinear           UMETA(DisplayName = "Bilinear (Smooth)"),
    Bicubic            UMETA(DisplayName = "Bicubic (High Quality)"),
    Lanczos            UMETA(DisplayName = "Lanczos (Best Quality)")
};

/**
 * Metadata extracted from DEM file
 */
USTRUCT(BlueprintType)
struct FDEMMetadata
{
    GENERATED_BODY()
    
    /** Width of DEM in pixels */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    int32 Width = 0;
    
    /** Height of DEM in pixels */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    int32 Height = 0;
    
    /** Minimum elevation value in meters */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    float MinElevation = 0.0f;
    
    /** Maximum elevation value in meters */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    float MaxElevation = 0.0f;
    
    /** Southwest corner (min longitude, min latitude) */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    FVector2D LatLonMin = FVector2D::ZeroVector;
    
    /** Northeast corner (max longitude, max latitude) */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    FVector2D LatLonMax = FVector2D::ZeroVector;
    
    /** Projection system (e.g., "WGS84", "UTM Zone 12N") */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    FString ProjectionSystem = TEXT("Unknown");
    
    /** Resolution: meters per pixel */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    float MetersPerPixel = 0.0f;
    
    /** NoData value used in source file */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    float NoDataValue = -32768.0f;
    
    /** Whether NoData value is defined */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    bool bHasNoDataValue = false;
    
    /** Source filename */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    FString SourceFile = TEXT("");
    
    /** Geographic center point */
    UPROPERTY(BlueprintReadOnly, Category = "DEM Metadata")
    FVector2D CenterLatLon = FVector2D::ZeroVector;
    
    FDEMMetadata() { }
    
    /** Calculate center point from bounds */
    void CalculateCenterPoint()
    {
        CenterLatLon = (LatLonMin + LatLonMax) * 0.5f;
    }
    
    /** Get human-readable description */
    FString ToString() const
    {
        return FString::Printf(
            TEXT("DEM: %dx%d pixels, %.1fm to %.1fm elevation, Center: (%.6f, %.6f)"),
            Width, Height, MinElevation, MaxElevation,
            CenterLatLon.X, CenterLatLon.Y
        );
    }
};

/**
 * Import settings for DEM processing
 */
USTRUCT(BlueprintType)
struct FDEMImportSettings
{
    GENERATED_BODY()
    
    /** Target width for resampling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    int32 TargetWidth = 513;
    
    /** Target height for resampling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    int32 TargetHeight = 513;
    
    /** Resampling method */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    EDEMResampleMethod ResampleMethod = EDEMResampleMethod::Bilinear;
    
    /** Scale factor for elevation values (multiplier) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    float ElevationScale = 1.0f;
    
    /** Vertical offset to add to all elevations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    float ElevationOffset = 0.0f;
    
    /** Whether to normalize elevation to 0-MaxHeight range */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    bool bNormalizeElevation = true;
    
    /** Target maximum height after normalization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    float NormalizedMaxHeight = 2000.0f;
    
    /** Whether to invert Z-axis (some DEMs have inverted elevation) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    bool bInvertZ = false;
    
    /** How to handle NoData values */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    bool bInterpolateNoData = true;
    
    /** Fill value for NoData (if not interpolating) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Import Settings")
    float NoDataFillValue = 0.0f;
};

/**
 * Digital Elevation Model Importer - UE5 Native Implementation
 *
 * Loads real-world terrain data using only UE5 built-in modules:
 * - SRTM HGT (NASA Shuttle Radar Topography Mission)
 * - ESRI ASCII Grid
 * - 16-bit PNG heightmaps (via IImageWrapper)
 * - TIFF with world files (via IImageWrapper)
 * - Raw binary data
 *
 * Features:
 * - Geographic coordinate support (lat/lon)
 * - Automatic resampling to target resolution
 * - Multiple interpolation methods
 * - NoData handling
 * - Elevation normalization
 * - Zero external dependencies
 */
UCLASS(BlueprintType)
class DRIFT_API UDEMImporter : public UObject
{
    GENERATED_BODY()
    
public:
    UDEMImporter();
    
    // ===== MAIN IMPORT FUNCTIONS =====
    
    /**
     * Import DEM from file with automatic format detection
     * @param FilePath - Full path to DEM file
     * @return true if import successful
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    bool ImportDEM(const FString& FilePath);
    
    /**
     * Import DEM from file with specified format
     * @param FilePath - Full path to DEM file
     * @param Format - File format to use
     * @return true if import successful
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    bool ImportDEMWithFormat(const FString& FilePath, EDEMFormat Format);
    
    /**
     * Import DEM with custom settings
     * @param FilePath - Full path to DEM file
     * @param Format - File format to use
     * @param Settings - Import and processing settings
     * @return true if import successful
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    bool ImportDEMWithSettings(const FString& FilePath, EDEMFormat Format,
                               const FDEMImportSettings& Settings);
    
    // ===== DATA ACCESS =====
    
    /**
     * Get raw height data array (original resolution)
     * @return Array of elevation values in meters
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    const TArray<float>& GetHeightData() const { return HeightData; }
    
    /**
     * Get metadata about loaded DEM
     * @return Metadata structure with dimensions, bounds, etc.
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    FDEMMetadata GetMetadata() const { return Metadata; }
    
    /**
     * Check if DEM is currently loaded
     * @return true if valid data loaded
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    bool IsLoaded() const { return HeightData.Num() > 0 && Metadata.Width > 0; }
    
    // ===== RESAMPLING =====
    
    /**
     * Resample DEM to target size
     * @param TargetWidth - Desired width in pixels
     * @param TargetHeight - Desired height in pixels
     * @param Method - Interpolation method to use
     * @return Resampled height array
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    TArray<float> ResampleToSize(int32 TargetWidth, int32 TargetHeight,
                                  EDEMResampleMethod Method = EDEMResampleMethod::Bilinear);
    
    /**
     * Resample to match terrain dimensions from MasterController
     * @param MasterController - Reference to get world dimensions
     * @return Resampled height array
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    TArray<float> ResampleToTerrainSize(class AMasterWorldController* MasterWorldController);
    
    // ===== COORDINATE CONVERSION =====
    
    /**
     * Convert latitude/longitude to grid coordinates
     * @param Latitude - Latitude in degrees
     * @param Longitude - Longitude in degrees
     * @return Grid position (X, Y) in pixels
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    FVector2D LatLonToGrid(float Latitude, float Longitude) const;
    
    /**
     * Convert grid coordinates to latitude/longitude
     * @param GridX - Grid X position in pixels
     * @param GridY - Grid Y position in pixels
     * @return Geographic coordinates (Longitude, Latitude)
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    FVector2D GridToLatLon(float GridX, float GridY) const;
    
    /**
     * Get elevation at specific lat/lon
     * @param Latitude - Latitude in degrees
     * @param Longitude - Longitude in degrees
     * @return Elevation in meters (interpolated)
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    float GetElevationAtLatLon(float Latitude, float Longitude) const;
    
    /**
     * Get elevation at grid position
     * @param GridX - Grid X position (can be fractional)
     * @param GridY - Grid Y position (can be fractional)
     * @return Elevation in meters (interpolated)
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    float GetElevationAtGrid(float GridX, float GridY) const;
    
    // ===== DATA PROCESSING =====
    
    /**
     * Normalize elevation data to 0 - MaxHeight range
     * @param MaxHeight - Target maximum height
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    void NormalizeElevation(float MaxHeight = 2000.0f);
    
    /**
     * Apply vertical exaggeration factor
     * @param Factor - Multiplier for elevation values
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    void ApplyVerticalExaggeration(float Factor);
    
    /**
     * Fill NoData values by interpolation
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    void InterpolateNoDataValues();
    
    /**
     * Apply smoothing filter to reduce noise
     * @param Iterations - Number of smoothing passes
     * @param Strength - Smoothing strength (0-1)
     */
    UFUNCTION(BlueprintCallable, Category = "DEM Import")
    void ApplySmoothing(int32 Iterations = 1, float Strength = 0.5f);
    
    // ===== UTILITY =====
    
    /**
     * Get format from file extension
     * @param FilePath - Path to file
     * @return Detected format
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    static EDEMFormat DetectFormatFromExtension(const FString& FilePath);
    
    /**
     * Validate file exists and is readable
     * @param FilePath - Path to file
     * @return true if file is valid
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    static bool ValidateDEMFile(const FString& FilePath);
    
    /**
     * Get human-readable format name
     * @param Format - Format enum
     * @return Display name
     */
    UFUNCTION(BlueprintPure, Category = "DEM Import")
    static FString GetFormatDisplayName(EDEMFormat Format);
    
private:
    // ===== DATA STORAGE =====
    
    /** Raw elevation data in meters */
    TArray<float> HeightData;
    
    /** DEM metadata */
    FDEMMetadata Metadata;
    
    /** Current import settings */
    FDEMImportSettings CurrentSettings;
    
    // ===== FORMAT-SPECIFIC LOADERS =====
    
    bool LoadSRTM_HGT(const FString& FilePath);
    bool LoadASCIIGrid(const FString& FilePath);
    bool LoadPNG16(const FString& FilePath);
    bool LoadTIFF(const FString& FilePath);
    bool LoadRAW(const FString& FilePath);
    
    // ===== TIFF WORLD FILE SUPPORT =====
    
    bool LoadTIFFWorldFile(const FString& TIFFPath);
    bool ParseWorldFile(const FString& WorldFilePath);
    
    // ===== RESAMPLING IMPLEMENTATIONS =====
    
    float SampleNearestNeighbor(const TArray<float>& Data, int32 Width, int32 Height,
                                 float X, float Y) const;
    
    float SampleBilinear(const TArray<float>& Data, int32 Width, int32 Height,
                         float X, float Y) const;
    
    float SampleBicubic(const TArray<float>& Data, int32 Width, int32 Height,
                        float X, float Y) const;
    
    float SampleLanczos(const TArray<float>& Data, int32 Width, int32 Height,
                        float X, float Y) const;
    
    // ===== INTERPOLATION HELPERS =====
    
    float CubicInterpolate(float p0, float p1, float p2, float p3, float t) const;
    float LanczosKernel(float x, int32 a = 3) const;
    
    // ===== DATA VALIDATION =====
    
    bool ValidateHeightData() const;
    void FixInvalidValues();
    bool IsNoDataValue(float Value) const;
    
    // ===== COORDINATE HELPERS =====
    
    bool IsGeographicCoordinate(float Lon, float Lat) const;
    FVector2D ClampToGeographicBounds(FVector2D LatLon) const;
};
