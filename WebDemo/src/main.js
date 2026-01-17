import { Scene } from './rendering/scene.js';
import { Terrain, TerrainPreset } from './simulation/terrain.js';
import { Water } from './simulation/water.js';
import { TerrainMesh } from './rendering/terrainMesh.js';
import { WaterMesh } from './rendering/waterMesh.js';
import { SpringMarkers } from './rendering/springMarkers.js';
import { TerrainControls } from './ui/controls.js';

class App {
  constructor() {
    // Get container
    this.container = document.getElementById('app');

    // Create systems
    this.scene = new Scene(this.container);
    this.terrain = new Terrain();
    this.water = new Water(this.terrain);
    this.terrainMesh = new TerrainMesh(this.terrain);
    this.waterMesh = new WaterMesh(this.water, this.terrain);
    this.springMarkers = new SpringMarkers(this.water, this.terrain);

    // Add meshes to scene
    this.scene.add(this.terrainMesh.object);
    this.scene.add(this.waterMesh.object);
    this.scene.add(this.springMarkers.object);

    // Create controls
    this.controls = new TerrainControls(this.scene, this.terrain, this.terrainMesh);
    this.controls.updateToolDisplay();

    // Connect water tools - use brush size and strength
    this.controls.onAddWater = (x, y) => {
      const amount = this.terrain.brushStrength * 50;  // Scale strength to total water volume
      const radius = this.terrain.brushRadius;
      this.water.addWater(x, y, amount, radius);
    };
    this.controls.onAddSpring = (x, y) => {
      const flowRate = this.terrain.brushStrength * 3;  // Scale strength to flow rate
      this.water.addSpring(x, y, flowRate);
    };
    this.controls.onRemoveSpring = (x, y) => {
      this.water.removeSpringNear(x, y, this.terrain.brushRadius);
    };
    this.controls.onClearWater = () => {
      this.water.depth.fill(0);
      this.water.waveEnergy.fill(0);
      this.water.dirty = true;
    };

    // Connect time controls
    this.controls.onTimeScaleChange = (delta) => {
      this.timeScale = Math.max(0.25, Math.min(20.0, this.timeScale + delta));
    };
    this.controls.onTogglePause = () => {
      this.paused = !this.paused;
    };

    // Override terrain reset to also regenerate springs
    const originalReset = this.terrain.reset.bind(this.terrain);
    this.terrain.reset = () => {
      originalReset();
      this.water.reset();
      this.water.autoGenerateSprings();
    };

    // Auto-generate springs on startup
    this.water.autoGenerateSprings();

    // Setup terrain preset buttons
    this.setupPresetButtons();

    // Simulation timing
    this.lastTime = performance.now();
    this.simAccumulator = 0;
    this.simStep = 1 / 60; // 60 simulation steps per second

    // Time control
    this.timeScale = 1.0;
    this.paused = false;

    // FPS tracking
    this.frameCount = 0;
    this.fpsTime = 0;
    this.currentFps = 60;

    // Start render loop
    this.animate();

    console.log('Drift Web Demo initialized');
    console.log('Grid size:', this.terrain.width, 'x', this.terrain.height);
  }

  setupPresetButtons() {
    const buttons = document.querySelectorAll('.preset-btn');
    buttons.forEach(btn => {
      btn.addEventListener('click', () => {
        // Update active state
        buttons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');

        // Generate new terrain with preset
        const preset = btn.dataset.preset;
        this.terrain.generate(preset);
        this.water.reset();
        this.water.autoGenerateSprings();
      });
    });
  }

  animate() {
    requestAnimationFrame(() => this.animate());

    // Calculate delta time
    const now = performance.now();
    const rawDt = (now - this.lastTime) / 1000;
    this.lastTime = now;

    // Apply time scale (0 if paused)
    const dt = this.paused ? 0 : rawDt * this.timeScale;

    // FPS calculation (always runs, uses raw dt)
    this.frameCount++;
    this.fpsTime += rawDt;
    if (this.fpsTime >= 0.5) {  // Update every 0.5 seconds
      this.currentFps = this.frameCount / this.fpsTime;
      this.frameCount = 0;
      this.fpsTime = 0;
      // Update stats display
      this.controls.updateStats(
        this.currentFps,
        this.water.getTotalWater(),
        this.water.springs.length,
        this.timeScale,
        this.paused
      );
    }

    // Accumulate time for fixed-step simulation
    this.simAccumulator += dt;

    // Run simulation steps (max 4 per frame to avoid spiral of death)
    let steps = 0;
    while (this.simAccumulator >= this.simStep && steps < 4) {
      this.water.simulate(this.simStep);
      this.simAccumulator -= this.simStep;
      steps++;
    }

    // Update camera (fly controls)
    this.controls.updateCamera(rawDt);  // Use raw dt so camera works when paused

    // Update meshes
    this.terrainMesh.update();
    this.waterMesh.update(dt);  // Pass dt for wave animation
    this.springMarkers.update(dt);  // Pass dt for fountain animation

    // Update scene (sun animation)
    this.scene.update(dt);

    // Render
    this.scene.render();
  }
}

// Start app when DOM is ready
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => new App());
} else {
  new App();
}
