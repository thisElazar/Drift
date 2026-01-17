import { Scene } from './rendering/scene.js';
import { Terrain } from './simulation/terrain.js';
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

    // Connect water tools
    this.controls.onAddWater = (x, y) => {
      this.water.addWater(x, y, 15, 4);
    };
    this.controls.onAddSpring = (x, y) => {
      this.water.addSpring(x, y, 2);
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

    // Simulation timing
    this.lastTime = performance.now();
    this.simAccumulator = 0;
    this.simStep = 1 / 60; // 60 simulation steps per second

    // Start render loop
    this.animate();

    console.log('Drift Web Demo initialized');
    console.log('Grid size:', this.terrain.width, 'x', this.terrain.height);
  }

  animate() {
    requestAnimationFrame(() => this.animate());

    // Calculate delta time
    const now = performance.now();
    const dt = (now - this.lastTime) / 1000;
    this.lastTime = now;

    // Accumulate time for fixed-step simulation
    this.simAccumulator += dt;

    // Run simulation steps (max 4 per frame to avoid spiral of death)
    let steps = 0;
    while (this.simAccumulator >= this.simStep && steps < 4) {
      this.water.simulate(this.simStep);
      this.simAccumulator -= this.simStep;
      steps++;
    }

    // Update meshes
    this.terrainMesh.update();
    this.waterMesh.update();
    this.springMarkers.update();

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
