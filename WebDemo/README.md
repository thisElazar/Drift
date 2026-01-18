# Drift Web Demo

A browser-based water simulation demo showcasing terrain manipulation and fluid dynamics. This is a standalone web version of the Drift watershed simulator.

## Features

- **Real-time water simulation** - Fluid dynamics with flow, waves, and erosion
- **Interactive terrain editing** - Raise, lower, and sculpt the landscape
- **Multiple terrain presets** - Default, Hills, Mountains, Plains
- **Three arcade game modes** - Test your skills against the water
- **Unified desktop/mobile support** - Works on any device with automatic input detection
- **Adaptive performance** - Automatically adjusts quality for smooth gameplay

## Game Modes

### Sandbox
Free-form exploration with no objectives. Experiment with terrain and water to your heart's content.

### Flow Rush
Guide water to target zones before time runs out. Build combos by hitting targets in quick succession.

### Flood Defense
Protect villages from rising floodwaters. Springs spawn on high ground and flood downhill - build barriers to save the villagers.

### Cascade
Create a water path through a series of rings in order. Complete the cascade before time expires.

## Controls

### Desktop
- **Left-click** - Use current tool
- **Right-drag** - Rotate camera
- **Scroll** - Zoom in/out
- **WASD** - Pan camera
- **Q/E** - Move camera up/down
- **[ ]** - Adjust brush size
- **+/-** - Adjust brush strength
- **< >** - Adjust time scale
- **1-5** - Select tools
- **P** - Pause
- **R** - Reset terrain
- **C** - Clear water
- **H** - Reset camera view

### Mobile/Touch
- **Tap** - Use current tool
- **Two-finger drag** - Pan camera
- **Pinch** - Zoom in/out
- **Two-finger rotate** - Rotate camera

## Tools

1. **Raise** - Build up terrain
2. **Lower** - Dig into terrain
3. **Water** - Add water splash
4. **Spring** - Create permanent water source
5. **Remove** - Delete springs

## Quality Settings

- **Low** - 128x128 grid, best for mobile devices
- **Medium** - 192x192 grid, balanced performance
- **High** - 256x256 grid, full detail for desktop

Quality is auto-detected based on device type but can be changed manually.

## Development

### Prerequisites
- Node.js 18+
- npm

### Setup
```bash
cd WebDemo
npm install
```

### Run Development Server
```bash
npm run dev
```

### Build for Production
```bash
# Build unified version (recommended)
npm run build:unified

# Build desktop-only version
npm run build:desktop

# Build mobile-only version
npm run build:mobile

# Build all versions
npm run build:all
```

### Output
Built files are placed in:
- `dist-unified/` - Unified desktop/mobile build
- `dist/` - Desktop build
- `dist-mobile/` - Mobile build

## Technical Details

- Built with [Three.js](https://threejs.org/) for 3D rendering
- Custom shallow water simulation with wave propagation
- Spatial partitioning for performance optimization
- Adaptive frame rate management
- No external game engine dependencies

## License

Part of the Drift project. See main repository for license details.
