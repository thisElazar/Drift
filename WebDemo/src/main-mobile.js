import { Scene } from './rendering/scene.js';
import { Terrain } from './simulation/terrain.js';
import { Water } from './simulation/water.js';
import { TerrainMesh } from './rendering/terrainMesh.js';
import { WaterMesh } from './rendering/waterMesh.js';
import { SpringMarkers } from './rendering/springMarkers.js';
import { MobileControls } from './ui/mobileControls.js';
import {
  QualityLevel,
  QualityConfig,
  QualityPresets,
  setQuality,
  loadSavedQuality,
  autoDetectQuality
} from './simulation/quality.js';
import { getMaxSimSteps } from './simulation/constants.js';

// Initialize quality BEFORE creating app
function initQuality() {
  // Try to load saved preference
  const saved = loadSavedQuality();
  if (saved) {
    console.log('Loaded saved quality:', saved);
    return saved;
  }

  // Auto-detect based on screen size
  const detected = autoDetectQuality();
  setQuality(detected);
  console.log('Auto-detected quality:', detected);
  return detected;
}

class MobileApp {
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

    // Create mobile controls
    this.controls = new MobileControls(this.scene, this.terrain, this.terrainMesh);

    // Connect water tools
    this.controls.onAddWater = (x, y) => {
      const amount = this.terrain.brushStrength * 50;
      const radius = this.terrain.brushRadius;
      this.water.addWater(x, y, amount, radius);
    };
    this.controls.onAddSpring = (x, y) => {
      const flowRate = this.terrain.brushStrength * 3;
      this.water.addSpring(x, y, flowRate);
    };
    this.controls.onTerrainReset = () => {
      this.water.reset();
      this.water.autoGenerateSprings();
    };

    // Override terrain reset to also reset water
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
    this.simStep = 1 / 60;

    // FPS tracking
    this.frameCount = 0;
    this.fpsTime = 0;
    this.currentFps = 60;

    // Setup quality selector UI
    this.setupQualitySelector();

    // Start render loop
    this.animate();

    // Show help on first visit
    this.showHelpIfFirstVisit();

    console.log('Drift Mobile Demo initialized');
    console.log('Quality:', QualityConfig.level, '- Grid:', QualityConfig.gridWidth + 'x' + QualityConfig.gridHeight);
  }

  setupQualitySelector() {
    const qualityBtn = document.getElementById('btn-quality');
    const qualityDropdown = document.getElementById('quality-dropdown');
    const qualityButtons = document.querySelectorAll('.quality-btn');

    // Update button text to show current quality
    if (qualityBtn) {
      const preset = QualityPresets[QualityConfig.level];
      qualityBtn.textContent = preset ? preset.label : 'Quality';
    }

    // Update active state in dropdown
    qualityButtons.forEach(btn => {
      const level = btn.dataset.quality;
      btn.classList.toggle('active', level === QualityConfig.level);
    });

    // Toggle dropdown - use touchend to avoid conflicts
    if (qualityBtn && qualityDropdown) {
      const toggleDropdown = (e) => {
        e.preventDefault();
        e.stopPropagation();
        qualityDropdown.classList.toggle('visible');
        // Hide other dropdowns
        document.getElementById('preset-dropdown')?.classList.remove('visible');
      };
      qualityBtn.addEventListener('touchend', toggleDropdown);
      qualityBtn.addEventListener('click', (e) => {
        // Only handle click if not a touch device (prevents double-fire)
        if (!e.sourceCapabilities?.firesTouchEvents) {
          toggleDropdown(e);
        }
      });
    }

    // Handle quality selection - use touchend for better UX
    qualityButtons.forEach(btn => {
      const handleQuality = (e) => {
        e.preventDefault();
        e.stopPropagation();
        const level = btn.dataset.quality;

        if (level !== QualityConfig.level) {
          // Save new quality and reload to apply
          setQuality(level);
          window.location.reload();
        } else {
          qualityDropdown?.classList.remove('visible');
        }
      };
      btn.addEventListener('touchend', handleQuality);
      btn.addEventListener('click', (e) => {
        if (!e.sourceCapabilities?.firesTouchEvents) {
          handleQuality(e);
        }
      });
    });

    // Close dropdown when tapping elsewhere - use touchend with delay
    document.addEventListener('touchend', (e) => {
      // Small delay to let button handlers fire first
      setTimeout(() => {
        if (qualityDropdown?.classList.contains('visible')) {
          if (!qualityDropdown.contains(e.target) && e.target !== qualityBtn) {
            qualityDropdown.classList.remove('visible');
          }
        }
      }, 50);
    });
  }

  showHelpIfFirstVisit() {
    const hasVisited = localStorage.getItem('drift-mobile-visited');
    if (!hasVisited) {
      const helpOverlay = document.getElementById('help-overlay');
      if (helpOverlay) {
        helpOverlay.classList.add('visible');
      }
      localStorage.setItem('drift-mobile-visited', 'true');
    }
  }

  animate() {
    requestAnimationFrame(() => this.animate());

    // Calculate delta time
    const now = performance.now();
    const dt = (now - this.lastTime) / 1000;
    this.lastTime = now;

    // FPS calculation
    this.frameCount++;
    this.fpsTime += dt;
    if (this.fpsTime >= 0.5) {
      this.currentFps = this.frameCount / this.fpsTime;
      this.frameCount = 0;
      this.fpsTime = 0;
      // Update stats display
      this.controls.updateStats(
        this.currentFps,
        this.water.getTotalWater(),
        this.water.springs.length
      );
    }

    // Accumulate time for fixed-step simulation
    this.simAccumulator += dt;

    // Run simulation steps (use quality-based max steps)
    const maxSteps = getMaxSimSteps();
    let steps = 0;
    while (this.simAccumulator >= this.simStep && steps < maxSteps) {
      this.water.simulate(this.simStep);
      this.simAccumulator -= this.simStep;
      steps++;
    }

    // Update meshes
    this.terrainMesh.update();
    this.waterMesh.update(dt);
    this.springMarkers.update(dt);

    // Update scene (sun animation)
    this.scene.update(dt);

    // Render
    this.scene.render();
  }
}

// Initialize quality first, then start app
const currentQuality = initQuality();

// Start app when DOM is ready
if (document.readyState === 'loading') {
  document.addEventListener('DOMContentLoaded', () => new MobileApp());
} else {
  new MobileApp();
}
