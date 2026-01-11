# Drift

A real-time terrain simulation sandbox built in Unreal Engine 5.5.

## Overview

Drift is an interactive world-building simulation featuring dynamic terrain, water physics, atmospheric systems, and ecosystem modeling. Shape landscapes, watch water flow and erode terrain, control weather patterns, and observe geological processes unfold in real-time.

## Features

### Terrain System
- Chunk-based dynamic terrain generation
- GPU-accelerated displacement mapping
- DEM import support (SRTM, ESRI, PNG16, TIFF, RAW formats)
- Multiple visualization modes (Naturalistic, Topographic, Wireframe)

### Water Simulation
- Pressure-based water flow physics
- 6 wave formation types
- Real-time erosion and sediment transport
- GPU compute shader acceleration
- Springs and water sources

### Atmosphere
- Dynamic weather system
- Volumetric cloud rendering
- Orographic precipitation effects
- Temperature and wind simulation

### Geology
- Groundwater systems
- Rock type classification
- Geological timescale simulation

### Ecosystem
- Biome-based vegetation
- Environmental response systems

## Technical Stack

- **Engine:** Unreal Engine 5.5
- **Language:** C++ with Blueprints
- **GPU:** Custom compute shaders (HLSL/USF)
- **Architecture:** MasterController orchestration pattern with domain-specific controllers

## Project Structure

```
Source/
├── MasterController      # Central system orchestration
├── TerrainController     # User input and camera control
├── DynamicTerrain        # Terrain mesh and chunk management
├── WaterSystem           # Water simulation physics
├── WaterController       # Water editing interface
├── AtmosphereController  # Weather and atmosphere
├── AtmosphericSystem     # Atmosphere calculations
├── GeologyController     # Geological features
├── EcosystemController   # Vegetation and biomes
├── TemporalManager       # Time scaling and control
├── GPUComputeManager     # GPU resource management
├── DEMImporter           # Elevation data import
└── GamePreviewManager    # Menu world generation
```

## Building

1. Clone the repository
2. Open `TerrAI.uproject` in Unreal Engine 5.5
3. Build for your target platform

## Requirements

- Unreal Engine 5.5
- macOS, Windows, or Linux
- GPU with compute shader support

## Development Timeline

- **June 2025:** Initial terrain and water systems
- **July 2025:** MasterController architecture, geology systems
- **August 2025:** GPU compute shader transition
- **September 2025:** GPU infrastructure expansion
- **November 2025:** DEM import, volumetric clouds, production stabilization
- **December 2025:** Alpha preparation

## License

Proprietary - All rights reserved

## Author

Elazar Abraham
