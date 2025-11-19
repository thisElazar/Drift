// DEMImporter.cpp
// Digital Elevation Model Import System - UE5 Native Implementation
// Zero external dependencies - uses only UE5 built-in modules

#include "DEMImporter.h"
#include "MasterController.h"
#include "DynamicTerrain.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Modules/ModuleManager.h"

UDEMImporter::UDEMImporter()
{
    UE_LOG(LogTemp, Log, TEXT("DEMImporter: UE5-native implementation initialized"));
    UE_LOG(LogTemp, Log, TEXT("  Supported formats: SRTM HGT, ASCII Grid, PNG16, TIFF, RAW"));
}

// ===== MAIN IMPORT FUNCTIONS =====

bool UDEMImporter::ImportDEM(const FString& FilePath)
{
    EDEMFormat Format = DetectFormatFromExtension(FilePath);
    if (Format == EDEMFormat::Auto)
    {
        UE_LOG(LogTemp, Error, TEXT("Could not detect DEM format from extension: %s"), *FilePath);
        return false;
    }
    
    return ImportDEMWithFormat(FilePath, Format);
}

bool UDEMImporter::ImportDEMWithFormat(const FString& FilePath, EDEMFormat Format)
{
    FDEMImportSettings DefaultSettings;
    return ImportDEMWithSettings(FilePath, Format, DefaultSettings);
}

bool UDEMImporter::ImportDEMWithSettings(const FString& FilePath, EDEMFormat Format,
                                         const FDEMImportSettings& Settings)
{
    if (!ValidateDEMFile(FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid DEM file: %s"), *FilePath);
        return false;
    }
    
    CurrentSettings = Settings;
    Metadata.SourceFile = FilePath;
    
    UE_LOG(LogTemp, Warning, TEXT("=== Importing DEM ==="));
    UE_LOG(LogTemp, Warning, TEXT("File: %s"), *FPaths::GetCleanFilename(FilePath));
    UE_LOG(LogTemp, Warning, TEXT("Format: %s"), *GetFormatDisplayName(Format));
    
    bool bSuccess = false;
    
    switch (Format)
    {
        case EDEMFormat::SRTM_HGT:
            bSuccess = LoadSRTM_HGT(FilePath);
            break;
            
        case EDEMFormat::ASCIIGrid:
            bSuccess = LoadASCIIGrid(FilePath);
            break;
            
        case EDEMFormat::PNG16:
            bSuccess = LoadPNG16(FilePath);
            break;
            
        case EDEMFormat::TIFF:
            bSuccess = LoadTIFF(FilePath);
            break;
            
        case EDEMFormat::RAW:
            bSuccess = LoadRAW(FilePath);
            break;
            
        default:
            UE_LOG(LogTemp, Error, TEXT("Unsupported DEM format"));
            return false;
    }
    
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load DEM data"));
        return false;
    }
    
    // Post-processing
    if (CurrentSettings.bInterpolateNoData && Metadata.bHasNoDataValue)
    {
        UE_LOG(LogTemp, Log, TEXT("Interpolating NoData values..."));
        InterpolateNoDataValues();
    }
    
    if (CurrentSettings.bNormalizeElevation)
    {
        UE_LOG(LogTemp, Log, TEXT("Normalizing elevation to %.1fm..."), CurrentSettings.NormalizedMaxHeight);
        NormalizeElevation(CurrentSettings.NormalizedMaxHeight);
    }
    
    if (CurrentSettings.ElevationScale != 1.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Applying vertical exaggeration: %.2fx"), CurrentSettings.ElevationScale);
        ApplyVerticalExaggeration(CurrentSettings.ElevationScale);
    }
    
    if (CurrentSettings.ElevationOffset != 0.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("Applying elevation offset: %.1fm"), CurrentSettings.ElevationOffset);
        for (float& Height : HeightData)
        {
            Height += CurrentSettings.ElevationOffset;
        }
    }
    
    if (CurrentSettings.bInvertZ)
    {
        UE_LOG(LogTemp, Log, TEXT("Inverting Z-axis"));
        for (float& Height : HeightData)
        {
            Height = -Height;
        }
    }
    
    Metadata.CalculateCenterPoint();
    ValidateHeightData();
    
    UE_LOG(LogTemp, Warning, TEXT("=== DEM Import Complete ==="));
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Metadata.ToString());
    
    return true;
}

// ===== SRTM HGT LOADER =====

bool UDEMImporter::LoadSRTM_HGT(const FString& FilePath)
{
    // SRTM HGT format:
    // - Filename encodes location: N37W122.hgt = 37°N, 122°W
    // - 1° x 1° tiles
    // - 16-bit signed integers (big-endian)
    // - SRTM-1: 3601x3601 pixels (1 arc-second resolution ~30m)
    // - SRTM-3: 1201x1201 pixels (3 arc-second resolution ~90m)
    
    // Parse filename for coordinates
    FString Filename = FPaths::GetBaseFilename(FilePath);
    
    float Latitude = 0.0f;
    float Longitude = 0.0f;
    
    if (Filename.Len() >= 7)
    {
        // Example: "N37W122" → Lat=37°N, Lon=122°W
        FString LatStr = Filename.Mid(1, 2);
        FString LonStr = Filename.Mid(4, 3);
        
        Latitude = FCString::Atof(*LatStr);
        Longitude = FCString::Atof(*LonStr);
        
        if (Filename[0] == 'S') Latitude = -Latitude;
        if (Filename[3] == 'W') Longitude = -Longitude;
    }
    
    // Load file
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load HGT file: %s"), *FilePath);
        return false;
    }
    
    // Determine size from file size (each sample is 2 bytes)
    int32 NumSamples = FMath::FloorToInt(FMath::Sqrt(FileData.Num() / 2.0));
    
    if (NumSamples != 1201 && NumSamples != 3601)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unusual HGT size: %d (expected 1201 or 3601)"), NumSamples);
    }
    
    Metadata.Width = NumSamples;
    Metadata.Height = NumSamples;
    
    // Set geographic bounds
    Metadata.LatLonMin = FVector2D(Longitude, Latitude);
    Metadata.LatLonMax = FVector2D(Longitude + 1.0f, Latitude + 1.0f);
    
    // Calculate resolution
    if (NumSamples == 3601)
    {
        Metadata.MetersPerPixel = 30.0f; // SRTM-1: ~30m
    }
    else if (NumSamples == 1201)
    {
        Metadata.MetersPerPixel = 90.0f; // SRTM-3: ~90m
    }
    else
    {
        Metadata.MetersPerPixel = 111320.0f / NumSamples; // degrees to meters
    }
    
    Metadata.ProjectionSystem = TEXT("WGS84");
    Metadata.NoDataValue = -32768.0f;
    Metadata.bHasNoDataValue = true;
    
    // Read 16-bit big-endian data
    HeightData.SetNum(NumSamples * NumSamples);
    Metadata.MinElevation = FLT_MAX;
    Metadata.MaxElevation = -FLT_MAX;
    
    for (int32 i = 0; i < NumSamples * NumSamples; i++)
    {
        // Big-endian 16-bit signed integer
        int16 RawHeight = (FileData[i * 2] << 8) | FileData[i * 2 + 1];
        float Height = (float)RawHeight;
        
        if (RawHeight == -32768)
        {
            Height = 0.0f; // NoData - will interpolate later if requested
        }
        else
        {
            Metadata.MinElevation = FMath::Min(Metadata.MinElevation, Height);
            Metadata.MaxElevation = FMath::Max(Metadata.MaxElevation, Height);
        }
        
        HeightData[i] = Height;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded SRTM HGT: %s at (%.2f°, %.2f°)"),
           *Filename, Latitude, Longitude);
    UE_LOG(LogTemp, Log, TEXT("  Resolution: %dx%d (~%.0fm per pixel)"),
           NumSamples, NumSamples, Metadata.MetersPerPixel);
    
    return true;
}

// ===== ASCII GRID LOADER =====

bool UDEMImporter::LoadASCIIGrid(const FString& FilePath)
{
    // ESRI ASCII Grid format:
    // Header:
    //   ncols 512
    //   nrows 512
    //   xllcorner -122.5
    //   yllcorner 37.0
    //   cellsize 0.001
    //   NODATA_value -9999
    // Data: space-separated elevation values
    
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load ASCII Grid: %s"), *FilePath);
        return false;
    }
    
    TArray<FString> Lines;
    FileContent.ParseIntoArray(Lines, TEXT("\n"), true);
    
    // Parse header
    int32 NCols = 0, NRows = 0;
    float XLLCorner = 0.0f, YLLCorner = 0.0f, CellSize = 0.0f;
    float NoDataValue = -9999.0f;
    bool bHasNoData = false;
    int32 DataStartLine = 0;
    
    for (int32 i = 0; i < Lines.Num(); i++)
    {
        FString Line = Lines[i].TrimStartAndEnd();
        if (Line.IsEmpty()) continue;
        
        TArray<FString> Tokens;
        Line.ParseIntoArray(Tokens, TEXT(" "), true);
        
        if (Tokens.Num() < 2) continue;
        
        FString Key = Tokens[0].ToLower();
        float Value = FCString::Atof(*Tokens[1]);
        
        if (Key == TEXT("ncols")) NCols = (int32)Value;
        else if (Key == TEXT("nrows")) NRows = (int32)Value;
        else if (Key.Contains(TEXT("xllcorner"))) XLLCorner = Value;
        else if (Key.Contains(TEXT("yllcorner"))) YLLCorner = Value;
        else if (Key == TEXT("cellsize")) CellSize = Value;
        else if (Key.Contains(TEXT("nodata"))) { NoDataValue = Value; bHasNoData = true; }
        else { DataStartLine = i; break; } // End of header
    }
    
    if (NCols <= 0 || NRows <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid ASCII Grid header"));
        return false;
    }
    
    // Set metadata
    Metadata.Width = NCols;
    Metadata.Height = NRows;
    Metadata.LatLonMin = FVector2D(XLLCorner, YLLCorner);
    Metadata.LatLonMax = FVector2D(XLLCorner + NCols * CellSize, YLLCorner + NRows * CellSize);
    Metadata.MetersPerPixel = CellSize * 111320.0f; // degrees to meters
    Metadata.NoDataValue = NoDataValue;
    Metadata.bHasNoDataValue = bHasNoData;
    Metadata.ProjectionSystem = TEXT("WGS84");
    
    // Read data
    HeightData.SetNum(NCols * NRows);
    Metadata.MinElevation = FLT_MAX;
    Metadata.MaxElevation = -FLT_MAX;
    
    int32 HeightIndex = 0;
    
    for (int32 i = DataStartLine; i < Lines.Num() && HeightIndex < HeightData.Num(); i++)
    {
        FString Line = Lines[i].TrimStartAndEnd();
        if (Line.IsEmpty()) continue;
        
        TArray<FString> Values;
        Line.ParseIntoArray(Values, TEXT(" "), true);
        
        for (const FString& ValueStr : Values)
        {
            if (HeightIndex >= HeightData.Num()) break;
            
            float Height = FCString::Atof(*ValueStr);
            
            if (bHasNoData && FMath::IsNearlyEqual(Height, NoDataValue, 0.01f))
            {
                Height = 0.0f;
            }
            else
            {
                Metadata.MinElevation = FMath::Min(Metadata.MinElevation, Height);
                Metadata.MaxElevation = FMath::Max(Metadata.MaxElevation, Height);
            }
            
            HeightData[HeightIndex++] = Height;
        }
    }
    
    if (HeightIndex != HeightData.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("ASCII Grid data count mismatch: expected %d, got %d"),
               HeightData.Num(), HeightIndex);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded ASCII Grid: %dx%d, cellsize=%.6f"), NCols, NRows, CellSize);
    return true;
}

// ===== PNG16 LOADER (UE5 IImageWrapper) =====

bool UDEMImporter::LoadPNG16(const FString& FilePath)
{
    // Load PNG using UE5's built-in IImageWrapper
    TArray<uint8> RawFileData;
    if (!FFileHelper::LoadFileToArray(RawFileData, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load PNG file: %s"), *FilePath);
        return false;
    }
    
    // Use IImageWrapper to decode PNG
    IImageWrapperModule& ImageWrapperModule =
        FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    
    TSharedPtr<IImageWrapper> ImageWrapper =
        ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
    
    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to decompress PNG"));
        return false;
    }
    
    int32 BitDepth = ImageWrapper->GetBitDepth();
    
    TArray<uint8> RawData;
    if (!ImageWrapper->GetRaw(ERGBFormat::Gray, BitDepth, RawData))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get raw PNG data"));
        return false;
    }
    
    Metadata.Width = ImageWrapper->GetWidth();
    Metadata.Height = ImageWrapper->GetHeight();
    
    // Convert to float heights
    HeightData.SetNum(Metadata.Width * Metadata.Height);
    
    if (BitDepth == 16)
    {
        const uint16* Data16 = reinterpret_cast<const uint16*>(RawData.GetData());
        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            // Normalize 16-bit value (0-65535) to reasonable height range
            HeightData[i] = (float)Data16[i] / 65535.0f * 8848.0f; // Mt. Everest height
        }
    }
    else if (BitDepth == 8)
    {
        const uint8* Data8 = RawData.GetData();
        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            HeightData[i] = (float)Data8[i] / 255.0f * 8848.0f;
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unsupported PNG bit depth: %d"), BitDepth);
        return false;
    }
    
    // Calculate elevation range
    Metadata.MinElevation = FLT_MAX;
    Metadata.MaxElevation = -FLT_MAX;
    for (float Height : HeightData)
    {
        Metadata.MinElevation = FMath::Min(Metadata.MinElevation, Height);
        Metadata.MaxElevation = FMath::Max(Metadata.MaxElevation, Height);
    }
    
    // No geographic info in PNG - use pixel coordinates
    Metadata.LatLonMin = FVector2D(0, 0);
    Metadata.LatLonMax = FVector2D(Metadata.Width, Metadata.Height);
    Metadata.MetersPerPixel = 1.0f;
    Metadata.ProjectionSystem = TEXT("Pixel Space");
    
    UE_LOG(LogTemp, Log, TEXT("Loaded PNG16: %dx%d (%d-bit)"),
           Metadata.Width, Metadata.Height, BitDepth);
    
    return true;
}

// ===== TIFF LOADER (UE5 IImageWrapper) =====

bool UDEMImporter::LoadTIFF(const FString& FilePath)
{
    // Load TIFF using UE5's built-in IImageWrapper
    TArray<uint8> RawFileData;
    if (!FFileHelper::LoadFileToArray(RawFileData, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load TIFF file: %s"), *FilePath);
        return false;
    }
    
    // Use IImageWrapper to decode TIFF
    IImageWrapperModule& ImageWrapperModule =
        FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    
    TSharedPtr<IImageWrapper> ImageWrapper =
        ImageWrapperModule.CreateImageWrapper(EImageFormat::TIFF);
    
    if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(RawFileData.GetData(), RawFileData.Num()))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to decompress TIFF"));
        return false;
    }
    
    int32 BitDepth = ImageWrapper->GetBitDepth();
    
    UE_LOG(LogTemp, Log, TEXT("TIFF bit depth: %d"), BitDepth);
    
    TArray<uint8> RawData;
    bool bSuccess = false;
    
    // Support 16-bit and 32-bit TIFF
    if (BitDepth == 16)
    {
        bSuccess = ImageWrapper->GetRaw(ERGBFormat::Gray, 16, RawData);
    }
    else if (BitDepth == 32)
    {
        // Try RGBA first (some TIFFs store elevation in RGBA channels)
        bSuccess = ImageWrapper->GetRaw(ERGBFormat::RGBA, 32, RawData);
        if (!bSuccess)
        {
            // Try BGRA
            bSuccess = ImageWrapper->GetRaw(ERGBFormat::BGRA, 32, RawData);
        }
        if (!bSuccess)
        {
            // Try Gray as last resort
            bSuccess = ImageWrapper->GetRaw(ERGBFormat::Gray, 32, RawData);
        }
    }
    else if (BitDepth == 8)
    {
        bSuccess = ImageWrapper->GetRaw(ERGBFormat::Gray, 8, RawData);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Unsupported TIFF bit depth: %d"), BitDepth);
        return false;
    }
    
    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get raw TIFF data"));
        return false;
    }
    
    Metadata.Width = ImageWrapper->GetWidth();
    Metadata.Height = ImageWrapper->GetHeight();
    
    // Convert to float heights
    HeightData.SetNum(Metadata.Width * Metadata.Height);
    
    if (BitDepth == 16)
    {
        const uint16* Data16 = reinterpret_cast<const uint16*>(RawData.GetData());
        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            HeightData[i] = (float)Data16[i] / 65535.0f * 8848.0f; // Scale to Mt. Everest height
        }
    }
    else if (BitDepth == 32)
    {
        // Assume float data
        const float* DataFloat = reinterpret_cast<const float*>(RawData.GetData());
        FMemory::Memcpy(HeightData.GetData(), DataFloat, HeightData.Num() * sizeof(float));
    }
    else if (BitDepth == 8)
    {
        const uint8* Data8 = RawData.GetData();
        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            HeightData[i] = (float)Data8[i] / 255.0f * 8848.0f;
        }
    }
    
    // Calculate elevation range
    Metadata.MinElevation = FLT_MAX;
    Metadata.MaxElevation = -FLT_MAX;
    for (float Height : HeightData)
    {
        Metadata.MinElevation = FMath::Min(Metadata.MinElevation, Height);
        Metadata.MaxElevation = FMath::Max(Metadata.MaxElevation, Height);
    }
    
    // Try to load geographic metadata from world file
    if (!LoadTIFFWorldFile(FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("No world file found, using pixel space"));
        Metadata.LatLonMin = FVector2D(0, 0);
        Metadata.LatLonMax = FVector2D(Metadata.Width, Metadata.Height);
        Metadata.MetersPerPixel = 1.0f;
        Metadata.ProjectionSystem = TEXT("Pixel Space");
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded TIFF: %dx%d (%d-bit)"),
           Metadata.Width, Metadata.Height, BitDepth);
    
    return true;
}

// ===== TIFF WORLD FILE SUPPORT =====

bool UDEMImporter::LoadTIFFWorldFile(const FString& TIFFPath)
{
    // Look for companion .tfw file (TIFF World File)
    FString TWFPath = FPaths::ChangeExtension(TIFFPath, TEXT("tfw"));
    
    if (!FPaths::FileExists(TWFPath))
    {
        // Try .tifw extension
        TWFPath = TIFFPath + TEXT("w");
        if (!FPaths::FileExists(TWFPath))
        {
            // Try .tif.world
            TWFPath = TIFFPath + TEXT(".world");
            if (!FPaths::FileExists(TWFPath))
            {
                return false;
            }
        }
    }
    
    return ParseWorldFile(TWFPath);
}

bool UDEMImporter::ParseWorldFile(const FString& WorldFilePath)
{
    // World File format:
    // Line 1: X pixel size (degrees or meters per pixel)
    // Line 2: Y rotation (usually 0)
    // Line 3: X rotation (usually 0)
    // Line 4: Y pixel size (negative, degrees or meters per pixel)
    // Line 5: X coordinate of upper-left pixel center
    // Line 6: Y coordinate of upper-left pixel center
    
    FString Content;
    if (!FFileHelper::LoadFileToString(Content, *WorldFilePath))
    {
        return false;
    }
    
    TArray<FString> Lines;
    Content.ParseIntoArray(Lines, TEXT("\n"), true);
    
    if (Lines.Num() < 6)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid world file format (need 6 lines)"));
        return false;
    }
    
    float PixelSizeX = FCString::Atof(*Lines[0].TrimStartAndEnd());
    float PixelSizeY = FCString::Atof(*Lines[3].TrimStartAndEnd()); // Usually negative
    float UpperLeftX = FCString::Atof(*Lines[4].TrimStartAndEnd());
    float UpperLeftY = FCString::Atof(*Lines[5].TrimStartAndEnd());
    
    // Calculate bounds
    Metadata.LatLonMin = FVector2D(
        UpperLeftX,
        UpperLeftY + PixelSizeY * Metadata.Height
    );
    
    Metadata.LatLonMax = FVector2D(
        UpperLeftX + PixelSizeX * Metadata.Width,
        UpperLeftY
    );
    
    // Determine if this is geographic or projected coordinates
    if (FMath::Abs(PixelSizeX) < 1.0f && FMath::Abs(PixelSizeY) < 1.0f)
    {
        // Likely geographic coordinates (degrees)
        Metadata.MetersPerPixel = FMath::Abs(PixelSizeX) * 111320.0f; // Approximate at equator
        Metadata.ProjectionSystem = TEXT("WGS84 (from world file)");
    }
    else
    {
        // Likely projected coordinates (meters)
        Metadata.MetersPerPixel = FMath::Abs(PixelSizeX);
        Metadata.ProjectionSystem = TEXT("Projected (from world file)");
    }
    
    UE_LOG(LogTemp, Log, TEXT("Loaded world file: %.6f, %.6f → %.6f, %.6f"),
           Metadata.LatLonMin.X, Metadata.LatLonMin.Y,
           Metadata.LatLonMax.X, Metadata.LatLonMax.Y);
    
    return true;
}

// ===== RAW LOADER =====

bool UDEMImporter::LoadRAW(const FString& FilePath)
{
    // Assume square RAW file with 32-bit floats
    TArray<uint8> FileData;
    if (!FFileHelper::LoadFileToArray(FileData, *FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load RAW file: %s"), *FilePath);
        return false;
    }
    
    int32 NumFloats = FileData.Num() / sizeof(float);
    int32 Size = FMath::FloorToInt(FMath::Sqrt((float)NumFloats));
    
    if (Size * Size != NumFloats)
    {
        UE_LOG(LogTemp, Error, TEXT("RAW file is not square: %d floats"), NumFloats);
        return false;
    }
    
    Metadata.Width = Size;
    Metadata.Height = Size;
    
    HeightData.SetNum(NumFloats);
    FMemory::Memcpy(HeightData.GetData(), FileData.GetData(), FileData.Num());
    
    // Calculate min/max
    Metadata.MinElevation = FLT_MAX;
    Metadata.MaxElevation = -FLT_MAX;
    for (float Height : HeightData)
    {
        Metadata.MinElevation = FMath::Min(Metadata.MinElevation, Height);
        Metadata.MaxElevation = FMath::Max(Metadata.MaxElevation, Height);
    }
    
    // No geographic info - use pixel coordinates
    Metadata.LatLonMin = FVector2D(0, 0);
    Metadata.LatLonMax = FVector2D(Size, Size);
    Metadata.MetersPerPixel = 1.0f;
    Metadata.ProjectionSystem = TEXT("Pixel Space");
    
    UE_LOG(LogTemp, Log, TEXT("Loaded RAW: %dx%d"), Size, Size);
    return true;
}

// ===== RESAMPLING =====

TArray<float> UDEMImporter::ResampleToSize(int32 TargetWidth, int32 TargetHeight,
                                            EDEMResampleMethod Method)
{
    if (!IsLoaded())
    {
        UE_LOG(LogTemp, Error, TEXT("No DEM loaded for resampling"));
        return TArray<float>();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Resampling %dx%d → %dx%d (%s)"),
           Metadata.Width, Metadata.Height,
           TargetWidth, TargetHeight,
           *UEnum::GetValueAsString(Method));
    
    TArray<float> Resampled;
    Resampled.SetNum(TargetWidth * TargetHeight);
    
    float ScaleX = (float)(Metadata.Width - 1) / FMath::Max(1, TargetWidth - 1);
    float ScaleY = (float)(Metadata.Height - 1) / FMath::Max(1, TargetHeight - 1);
    
    for (int32 Y = 0; Y < TargetHeight; Y++)
    {
        for (int32 X = 0; X < TargetWidth; X++)
        {
            float SrcX = X * ScaleX;
            float SrcY = Y * ScaleY;
            
            float Height = 0.0f;
            
            switch (Method)
            {
                case EDEMResampleMethod::NearestNeighbor:
                    Height = SampleNearestNeighbor(HeightData, Metadata.Width, Metadata.Height, SrcX, SrcY);
                    break;
                    
                case EDEMResampleMethod::Bilinear:
                    Height = SampleBilinear(HeightData, Metadata.Width, Metadata.Height, SrcX, SrcY);
                    break;
                    
                case EDEMResampleMethod::Bicubic:
                    Height = SampleBicubic(HeightData, Metadata.Width, Metadata.Height, SrcX, SrcY);
                    break;
                    
                case EDEMResampleMethod::Lanczos:
                    Height = SampleLanczos(HeightData, Metadata.Width, Metadata.Height, SrcX, SrcY);
                    break;
            }
            
            Resampled[Y * TargetWidth + X] = Height;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Resampling complete"));
    return Resampled;
}

TArray<float> UDEMImporter::ResampleToTerrainSize(AMasterWorldController* MasterWorldController)
{
    if (!MasterWorldController || !MasterWorldController->MainTerrain)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid MasterController reference"));
        return TArray<float>();
    }
    
    int32 TerrainWidth = MasterWorldController->MainTerrain->TerrainWidth;
    int32 TerrainHeight = MasterWorldController->MainTerrain->TerrainHeight;

    
    return ResampleToSize(TerrainWidth, TerrainHeight, CurrentSettings.ResampleMethod);
}

// ===== RESAMPLING IMPLEMENTATIONS =====

float UDEMImporter::SampleNearestNeighbor(const TArray<float>& Data, int32 Width, int32 Height,
                                          float X, float Y) const
{
    int32 IX = FMath::RoundToInt(X);
    int32 IY = FMath::RoundToInt(Y);
    
    IX = FMath::Clamp(IX, 0, Width - 1);
    IY = FMath::Clamp(IY, 0, Height - 1);
    
    return Data[IY * Width + IX];
}

float UDEMImporter::SampleBilinear(const TArray<float>& Data, int32 Width, int32 Height,
                                    float X, float Y) const
{
    int32 X0 = FMath::FloorToInt(X);
    int32 Y0 = FMath::FloorToInt(Y);
    int32 X1 = X0 + 1;
    int32 Y1 = Y0 + 1;
    
    X0 = FMath::Clamp(X0, 0, Width - 1);
    X1 = FMath::Clamp(X1, 0, Width - 1);
    Y0 = FMath::Clamp(Y0, 0, Height - 1);
    Y1 = FMath::Clamp(Y1, 0, Height - 1);
    
    float FracX = X - X0;
    float FracY = Y - Y0;
    
    float V00 = Data[Y0 * Width + X0];
    float V10 = Data[Y0 * Width + X1];
    float V01 = Data[Y1 * Width + X0];
    float V11 = Data[Y1 * Width + X1];
    
    float V0 = FMath::Lerp(V00, V10, FracX);
    float V1 = FMath::Lerp(V01, V11, FracX);
    
    return FMath::Lerp(V0, V1, FracY);
}

float UDEMImporter::SampleBicubic(const TArray<float>& Data, int32 Width, int32 Height,
                                   float X, float Y) const
{
    int32 X1 = FMath::FloorToInt(X);
    int32 Y1 = FMath::FloorToInt(Y);
    
    float FracX = X - X1;
    float FracY = Y - Y1;
    
    // Sample 4x4 neighborhood
    float Rows[4];
    for (int32 j = 0; j < 4; j++)
    {
        int32 Y_Sample = FMath::Clamp(Y1 - 1 + j, 0, Height - 1);
        
        float Cols[4];
        for (int32 i = 0; i < 4; i++)
        {
            int32 X_Sample = FMath::Clamp(X1 - 1 + i, 0, Width - 1);
            Cols[i] = Data[Y_Sample * Width + X_Sample];
        }
        
        Rows[j] = CubicInterpolate(Cols[0], Cols[1], Cols[2], Cols[3], FracX);
    }
    
    return CubicInterpolate(Rows[0], Rows[1], Rows[2], Rows[3], FracY);
}

float UDEMImporter::SampleLanczos(const TArray<float>& Data, int32 Width, int32 Height,
                                   float X, float Y) const
{
    const int32 a = 3; // Lanczos kernel size
    
    int32 X0 = FMath::FloorToInt(X);
    int32 Y0 = FMath::FloorToInt(Y);
    
    float Sum = 0.0f;
    float WeightSum = 0.0f;
    
    for (int32 j = -a + 1; j <= a; j++)
    {
        for (int32 i = -a + 1; i <= a; i++)
        {
            int32 SampleX = FMath::Clamp(X0 + i, 0, Width - 1);
            int32 SampleY = FMath::Clamp(Y0 + j, 0, Height - 1);
            
            float dx = X - (X0 + i);
            float dy = Y - (Y0 + j);
            
            float Weight = LanczosKernel(dx, a) * LanczosKernel(dy, a);
            
            Sum += Data[SampleY * Width + SampleX] * Weight;
            WeightSum += Weight;
        }
    }
    
    return (WeightSum > 0.0f) ? (Sum / WeightSum) : 0.0f;
}

// ===== INTERPOLATION HELPERS =====

float UDEMImporter::CubicInterpolate(float p0, float p1, float p2, float p3, float t) const
{
    // Catmull-Rom spline
    float a = -0.5f * p0 + 1.5f * p1 - 1.5f * p2 + 0.5f * p3;
    float b = p0 - 2.5f * p1 + 2.0f * p2 - 0.5f * p3;
    float c = -0.5f * p0 + 0.5f * p2;
    float d = p1;
    
    return a * t * t * t + b * t * t + c * t + d;
}

float UDEMImporter::LanczosKernel(float x, int32 a) const
{
    if (x == 0.0f) return 1.0f;
    if (FMath::Abs(x) >= a) return 0.0f;
    
    float PiX = PI * x;
    return (a * FMath::Sin(PiX) * FMath::Sin(PiX / a)) / (PiX * PiX);
}

// ===== COORDINATE CONVERSION =====

FVector2D UDEMImporter::LatLonToGrid(float Latitude, float Longitude) const
{
    if (!IsLoaded())
    {
        return FVector2D::ZeroVector;
    }
    
    // Convert lat/lon to normalized position (0-1)
    float NormX = (Longitude - Metadata.LatLonMin.X) /
                  (Metadata.LatLonMax.X - Metadata.LatLonMin.X);
    float NormY = (Latitude - Metadata.LatLonMin.Y) /
                  (Metadata.LatLonMax.Y - Metadata.LatLonMin.Y);
    
    // Convert to grid coordinates
    float GridX = NormX * (Metadata.Width - 1);
    float GridY = NormY * (Metadata.Height - 1);
    
    return FVector2D(GridX, GridY);
}

FVector2D UDEMImporter::GridToLatLon(float GridX, float GridY) const
{
    if (!IsLoaded())
    {
        return FVector2D::ZeroVector;
    }
    
    // Normalize grid position
    float NormX = GridX / FMath::Max(1.0f, (float)(Metadata.Width - 1));
    float NormY = GridY / FMath::Max(1.0f, (float)(Metadata.Height - 1));
    
    // Convert to lat/lon
    float Longitude = FMath::Lerp(Metadata.LatLonMin.X, Metadata.LatLonMax.X, NormX);
    float Latitude = FMath::Lerp(Metadata.LatLonMin.Y, Metadata.LatLonMax.Y, NormY);
    
    return FVector2D(Longitude, Latitude);
}

float UDEMImporter::GetElevationAtLatLon(float Latitude, float Longitude) const
{
    FVector2D GridPos = LatLonToGrid(Latitude, Longitude);
    return GetElevationAtGrid(GridPos.X, GridPos.Y);
}

float UDEMImporter::GetElevationAtGrid(float GridX, float GridY) const
{
    if (!IsLoaded())
    {
        return 0.0f;
    }
    
    // Use bilinear interpolation for smooth elevation queries
    return SampleBilinear(HeightData, Metadata.Width, Metadata.Height, GridX, GridY);
}

// ===== DATA PROCESSING =====

void UDEMImporter::NormalizeElevation(float MaxHeight)
{
    if (!IsLoaded())
    {
        return;
    }
    
    float Range = Metadata.MaxElevation - Metadata.MinElevation;
    if (Range <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot normalize - elevation range is zero"));
        return;
    }
    
    float OldMin = Metadata.MinElevation;
    float OldMax = Metadata.MaxElevation;
    
    for (float& Height : HeightData)
    {
        Height = ((Height - Metadata.MinElevation) / Range) * MaxHeight;
    }
    
    Metadata.MinElevation = 0.0f;
    Metadata.MaxElevation = MaxHeight;
    
    UE_LOG(LogTemp, Log, TEXT("Normalized elevation: [%.1f, %.1f] → [0, %.1f]"),
           OldMin, OldMax, MaxHeight);
}

void UDEMImporter::ApplyVerticalExaggeration(float Factor)
{
    if (!IsLoaded())
    {
        return;
    }
    
    for (float& Height : HeightData)
    {
        Height *= Factor;
    }
    
    Metadata.MinElevation *= Factor;
    Metadata.MaxElevation *= Factor;
    
    UE_LOG(LogTemp, Log, TEXT("Applied vertical exaggeration: %.2fx"), Factor);
}

void UDEMImporter::InterpolateNoDataValues()
{
    if (!IsLoaded() || !Metadata.bHasNoDataValue)
    {
        return;
    }
    
    int32 InterpolatedCount = 0;
    
    // Simple interpolation: average of valid neighbors
    TArray<float> Original = HeightData;
    
    for (int32 Y = 0; Y < Metadata.Height; Y++)
    {
        for (int32 X = 0; X < Metadata.Width; X++)
        {
            int32 Index = Y * Metadata.Width + X;
            
            if (IsNoDataValue(Original[Index]))
            {
                float Sum = 0.0f;
                int32 Count = 0;
                
                // Sample 8 neighbors
                for (int32 dy = -1; dy <= 1; dy++)
                {
                    for (int32 dx = -1; dx <= 1; dx++)
                    {
                        if (dx == 0 && dy == 0) continue;
                        
                        int32 NX = X + dx;
                        int32 NY = Y + dy;
                        
                        if (NX >= 0 && NX < Metadata.Width && NY >= 0 && NY < Metadata.Height)
                        {
                            int32 NIndex = NY * Metadata.Width + NX;
                            if (!IsNoDataValue(Original[NIndex]))
                            {
                                Sum += Original[NIndex];
                                Count++;
                            }
                        }
                    }
                }
                
                if (Count > 0)
                {
                    HeightData[Index] = Sum / Count;
                    InterpolatedCount++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Interpolated %d NoData values"), InterpolatedCount);
}

void UDEMImporter::ApplySmoothing(int32 Iterations, float Strength)
{
    if (!IsLoaded())
    {
        return;
    }
    
    Strength = FMath::Clamp(Strength, 0.0f, 1.0f);
    
    for (int32 Iter = 0; Iter < Iterations; Iter++)
    {
        TArray<float> Smoothed = HeightData;
        
        for (int32 Y = 1; Y < Metadata.Height - 1; Y++)
        {
            for (int32 X = 1; X < Metadata.Width - 1; X++)
            {
                int32 Index = Y * Metadata.Width + X;
                
                // 3x3 kernel average
                float Sum = 0.0f;
                for (int32 dy = -1; dy <= 1; dy++)
                {
                    for (int32 dx = -1; dx <= 1; dx++)
                    {
                        Sum += HeightData[(Y + dy) * Metadata.Width + (X + dx)];
                    }
                }
                
                float Smoothed_Value = Sum / 9.0f;
                Smoothed[Index] = FMath::Lerp(HeightData[Index], Smoothed_Value, Strength);
            }
        }
        
        HeightData = Smoothed;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied smoothing: %d iterations, strength=%.2f"),
           Iterations, Strength);
}

// ===== DATA VALIDATION =====

bool UDEMImporter::ValidateHeightData() const
{
    if (HeightData.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Height data is empty"));
        return false;
    }
    
    if (HeightData.Num() != Metadata.Width * Metadata.Height)
    {
        UE_LOG(LogTemp, Error, TEXT("Height data size mismatch: %d vs %d"),
               HeightData.Num(), Metadata.Width * Metadata.Height);
        return false;
    }
    
    // Check for invalid values
    int32 InvalidCount = 0;
    for (float Height : HeightData)
    {
        if (!FMath::IsFinite(Height))
        {
            InvalidCount++;
        }
    }
    
    if (InvalidCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Found %d invalid height values"), InvalidCount);
    }
    
    return true;
}

void UDEMImporter::FixInvalidValues()
{
    for (float& Height : HeightData)
    {
        if (!FMath::IsFinite(Height))
        {
            Height = 0.0f;
        }
    }
}

bool UDEMImporter::IsNoDataValue(float Value) const
{
    if (!Metadata.bHasNoDataValue)
    {
        return false;
    }
    
    return FMath::IsNearlyEqual(Value, Metadata.NoDataValue, 0.01f);
}

// ===== COORDINATE HELPERS =====

bool UDEMImporter::IsGeographicCoordinate(float Lon, float Lat) const
{
    return (Lon >= -180.0f && Lon <= 180.0f && Lat >= -90.0f && Lat <= 90.0f);
}

FVector2D UDEMImporter::ClampToGeographicBounds(FVector2D LatLon) const
{
    return FVector2D(
        FMath::Clamp(LatLon.X, Metadata.LatLonMin.X, Metadata.LatLonMax.X),
        FMath::Clamp(LatLon.Y, Metadata.LatLonMin.Y, Metadata.LatLonMax.Y)
    );
}

// ===== UTILITY FUNCTIONS =====

EDEMFormat UDEMImporter::DetectFormatFromExtension(const FString& FilePath)
{
    FString Extension = FPaths::GetExtension(FilePath).ToLower();
    
    if (Extension == TEXT("hgt"))
    {
        return EDEMFormat::SRTM_HGT;
    }
    else if (Extension == TEXT("asc") || Extension == TEXT("grd"))
    {
        return EDEMFormat::ASCIIGrid;
    }
    else if (Extension == TEXT("png"))
    {
        return EDEMFormat::PNG16;
    }
    else if (Extension == TEXT("tif") || Extension == TEXT("tiff"))
    {
        return EDEMFormat::TIFF;
    }
    else if (Extension == TEXT("raw") || Extension == TEXT("r32"))
    {
        return EDEMFormat::RAW;
    }
    
    return EDEMFormat::Auto;
}

bool UDEMImporter::ValidateDEMFile(const FString& FilePath)
{
    if (FilePath.IsEmpty())
    {
        return false;
    }
    
    if (!FPaths::FileExists(FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("File does not exist: %s"), *FilePath);
        return false;
    }
    
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.FileExists(*FilePath))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot access file: %s"), *FilePath);
        return false;
    }
    
    int64 FileSize = PlatformFile.FileSize(*FilePath);
    if (FileSize <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("File is empty: %s"), *FilePath);
        return false;
    }
    
    return true;
}

FString UDEMImporter::GetFormatDisplayName(EDEMFormat Format)
{
    switch (Format)
    {
        case EDEMFormat::SRTM_HGT:
            return TEXT("SRTM Height (.hgt)");
        case EDEMFormat::ASCIIGrid:
            return TEXT("ESRI ASCII Grid (.asc)");
        case EDEMFormat::PNG16:
            return TEXT("16-bit PNG (.png)");
        case EDEMFormat::TIFF:
            return TEXT("TIFF (.tif, .tiff)");
        case EDEMFormat::RAW:
            return TEXT("Raw Binary (.raw)");
        case EDEMFormat::Auto:
            return TEXT("Auto-detect");
        default:
            return TEXT("Unknown");
    }
}
