# Drift Web Demo - Development Plan

## Overview
A browser-based demo of Drift running at 1/4 scale (128×128 grid) featuring terrain manipulation and water simulation with springs.

## Folder Structure
```
WebDemo/
├── public/
│   └── index.html          # Entry point
├── src/
│   ├── simulation/
│   │   ├── terrain.js      # Heightmap data + raise/lower
│   │   ├── water.js        # Water depth, flow, springs
│   │   └── constants.js    # Shared simulation params
│   ├── rendering/
│   │   ├── scene.js        # Three.js scene setup
│   │   ├── terrainMesh.js  # Terrain geometry + materials
│   │   └── waterMesh.js    # Water surface rendering
│   └── ui/
│       ├── controls.js     # Mouse/touch interactions
│       └── toolbar.js      # Tool selection UI
├── assets/                 # Textures (optional)
├── package.json
├── vite.config.js          # Build tool
└── PLAN.md
```

## Core Features

### 1. Terrain System
- **Grid**: 128×128 Float32Array heightmap
- **Raise/Lower**: Gaussian brush with configurable radius/strength
- **Reset**: Regenerate terrain with Perlin noise

### 2. Water System
- **Data**:
  - `waterDepth[128×128]` - depth at each cell
  - `velocityX[128×128]` - east/west flow
  - `velocityY[128×128]` - north/south flow
- **Physics**: Port the pressure-based 8-directional flow from `WaterSystem.cpp`
- **Springs**: Array of `{x, y, flowRate}` objects that add water each frame

### 3. Spring Mechanics
- **Auto-load**: On startup, place 3-5 springs at random elevated positions
- **Terrain Reset**: Randomize spring positions (find new local maxima)
- **Visual**: Small marker/particle at spring locations

### 4. Interactions
| Tool | Action |
|------|--------|
| Raise | Left-click/drag raises terrain |
| Lower | Right-click/drag lowers terrain |
| Add Water | Click to add water blob |
| Add Spring | Click to place permanent spring |

## Technical Stack
- **Three.js** - 3D rendering (PlaneGeometry with vertex displacement)
- **Vite** - Fast dev server & bundler
- **Vanilla JS** - No framework needed for this scale

## Algorithm Port: Pressure-Based Flow

From `WaterSystem.cpp` simplified:

```javascript
function simulateWaterStep(dt) {
  const flowRate = 0.05;

  for (let y = 1; y < height - 1; y++) {
    for (let x = 1; x < width - 1; x++) {
      const idx = y * width + x;
      const surfaceHeight = terrain[idx] + waterDepth[idx];

      // Check 8 neighbors
      for (const [dx, dy] of neighbors) {
        const nIdx = (y + dy) * width + (x + dx);
        const neighborSurface = terrain[nIdx] + waterDepth[nIdx];

        // Pressure difference drives flow
        const pressureDiff = surfaceHeight - neighborSurface;
        if (pressureDiff > 0) {
          const flow = Math.min(waterDepth[idx], pressureDiff * flowRate);
          // Transfer water to neighbor
          waterDepthNext[idx] -= flow;
          waterDepthNext[nIdx] += flow;
        }
      }
    }
  }
}
```

## Implementation Phases

### Phase 1: Foundation
- [ ] Set up Vite + Three.js project
- [ ] Create terrain heightmap with Perlin noise
- [ ] Render terrain as displaced PlaneGeometry
- [ ] Basic orbit camera controls

### Phase 2: Terrain Editing
- [ ] Implement raycasting for mouse → terrain position
- [ ] Gaussian brush for raise/lower
- [ ] Real-time mesh updates on edit

### Phase 3: Water Simulation
- [ ] Port pressure-based flow algorithm
- [ ] Water depth array + simulation loop
- [ ] Water mesh rendering (semi-transparent surface)
- [ ] Add water tool (click to add blob)

### Phase 4: Springs
- [ ] Spring data structure
- [ ] Auto-generate springs on terrain peaks
- [ ] Visual markers for springs
- [ ] Terrain reset randomizes springs

### Phase 5: Polish
- [ ] UI toolbar for tool selection
- [ ] Touch support for mobile
- [ ] Performance tuning (limit simulation steps)
- [ ] Optional: Simple erosion (sediment pickup/deposit)

## Performance Budget
| Resource | Target |
|----------|--------|
| Grid size | 128×128 (16K cells) |
| Simulation FPS | 30+ |
| Render FPS | 60 |
| Memory | < 10MB |
| Load time | < 2s |

## Estimated Effort
- **Phase 1-2**: Foundation + terrain editing
- **Phase 3-4**: Water + springs
- **Phase 5**: Polish

Total: A focused implementation with clear milestones.

## Notes
- No erosion/sediment in initial version (can add later)
- No GPU compute needed at 128×128 scale
- Double-buffering water arrays prevents read/write conflicts
