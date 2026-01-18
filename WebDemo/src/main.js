import { Scene } from './rendering/scene.js';
import { Terrain, TerrainPreset } from './simulation/terrain.js';
import { Water } from './simulation/water.js';
import { TerrainMesh } from './rendering/terrainMesh.js';
import { WaterMesh } from './rendering/waterMesh.js';
import { SpringMarkers } from './rendering/springMarkers.js';
import { TerrainControls } from './ui/controls.js';
import {
  QualityLevel,
  QualityConfig,
  QualityPresets,
  setQuality,
  loadSavedQuality,
  autoDetectQuality
} from './simulation/quality.js';
import { GameModeManager, GameModeType } from './game/GameModeManager.js';
import { ScoreManager } from './game/ScoreManager.js';
import { FlowRushMode } from './game/modes/FlowRushMode.js';
import { CascadeMode } from './game/modes/CascadeMode.js';
import { FloodDefenseMode } from './game/modes/FloodDefenseMode.js';

// Initialize quality BEFORE creating app
function initQuality() {
  const saved = loadSavedQuality();
  if (saved) {
    console.log('Loaded saved quality:', saved);
    return saved;
  }
  // Desktop defaults to high quality
  setQuality(QualityLevel.HIGH);
  console.log('Using default quality: high');
  return QualityLevel.HIGH;
}

const currentQuality = initQuality();

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

    // Create game mode manager
    this.gameModeManager = new GameModeManager(this.terrain, this.water, this.scene.scene);
    this.gameModeManager.registerMode(GameModeType.FLOW_RUSH, FlowRushMode);
    this.gameModeManager.registerMode(GameModeType.CASCADE, CascadeMode);
    this.gameModeManager.registerMode(GameModeType.FLOOD_DEFENSE, FloodDefenseMode);
    this.setupGameModeCallbacks();

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
      // Flow rate scales with both brush strength and size
      const flowRate = this.terrain.brushStrength * this.terrain.brushRadius * 0.5;
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

    // Setup quality selector
    this.setupQualitySelector();

    // Setup home button
    this.setupHomeButton();

    // Setup mode selector
    this.setupModeSelector();

    // Setup pre-game modal
    this.setupPregameModal();

    // Connect home key (H) to controls
    this.controls.onResetCamera = () => {
      this.scene.resetCamera();
    };

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

  setupQualitySelector() {
    const qualityBtn = document.getElementById('btn-quality');
    const qualityDropdown = document.getElementById('quality-dropdown');
    const qualityButtons = document.querySelectorAll('.quality-btn');

    if (!qualityBtn || !qualityDropdown) return;

    // Update button text to show current quality
    const preset = QualityPresets[QualityConfig.level];
    qualityBtn.textContent = preset ? preset.label : 'Quality';

    // Update active state in dropdown
    qualityButtons.forEach(btn => {
      const level = btn.dataset.quality;
      btn.classList.toggle('active', level === QualityConfig.level);
    });

    // Toggle dropdown
    qualityBtn.addEventListener('click', (e) => {
      e.stopPropagation();
      qualityDropdown.classList.toggle('visible');
    });

    // Handle quality selection
    qualityButtons.forEach(btn => {
      btn.addEventListener('click', (e) => {
        e.stopPropagation();
        const level = btn.dataset.quality;

        if (level !== QualityConfig.level) {
          this.changeQuality(level);

          // Update button text and active states
          const preset = QualityPresets[level];
          qualityBtn.textContent = preset ? preset.label : 'Quality';
          qualityButtons.forEach(b => {
            b.classList.toggle('active', b.dataset.quality === level);
          });
        }
        qualityDropdown.classList.remove('visible');
      });
    });

    // Close dropdown when clicking elsewhere
    document.addEventListener('click', () => {
      qualityDropdown.classList.remove('visible');
    });
  }

  /**
   * Change quality without page reload - preserves terrain and water state
   */
  changeQuality(level) {
    // End any active game mode first
    if (this.gameModeManager.isPlaying) {
      this.gameModeManager.endMode(false);
      this.showArcadeHud(false);
    }

    // Take snapshots of current state
    const terrainSnapshot = this.terrain.getSnapshot();
    const waterSnapshot = this.water.getSnapshot();

    // Apply new quality settings
    setQuality(level);

    // Recreate terrain with new grid size
    this.terrain = new Terrain();
    this.terrain.loadFromSnapshot(terrainSnapshot);

    // Recreate water with new grid size
    this.water = new Water(this.terrain);
    this.water.loadFromSnapshot(waterSnapshot);

    // Rebuild meshes with new dimensions
    this.terrainMesh.terrain = this.terrain;
    this.terrainMesh.rebuild();

    this.waterMesh.water = this.water;
    this.waterMesh.terrain = this.terrain;
    this.waterMesh.rebuild();

    this.springMarkers.water = this.water;
    this.springMarkers.terrain = this.terrain;
    this.springMarkers.refreshScale();

    // Update controls with new terrain reference
    this.controls.terrain = this.terrain;
    this.controls.terrainMesh = this.terrainMesh;

    // Update game mode manager with new references
    this.gameModeManager.terrain = this.terrain;
    this.gameModeManager.water = this.water;

    console.log(`Quality changed to ${level}. Grid size: ${this.terrain.width}x${this.terrain.height}`);
  }

  setupHomeButton() {
    const homeBtn = document.getElementById('btn-home');
    if (homeBtn) {
      homeBtn.addEventListener('click', () => {
        this.scene.resetCamera();
      });
    }
  }

  // Mode descriptions for pre-game modal
  getModeInfo(modeType) {
    const modeInfo = {
      flowRush: {
        icon: '🎯',
        title: 'Flow Rush',
        subtitle: '2 minutes to score big',
        description: 'Guide water to glowing targets scattered across the terrain. Build channels and redirect springs to maximize your score!',
        objectives: [
          'Water touching targets earns points',
          'Chain hits to build your combo multiplier',
          'Use terrain tools to guide water flow',
          'Find all targets before time runs out'
        ]
      },
      cascade: {
        icon: '💧',
        title: 'Cascade',
        subtitle: 'Chain reactions for massive combos',
        description: 'Water must flow through rings in sequence. Each ring you hit adds to your combo - miss the sequence and start over!',
        objectives: [
          'Guide water through the highlighted ring',
          'Rings must be hit in order (follow the sequence)',
          'Longer chains = bigger score multipliers',
          'Plan your terrain to create smooth water paths'
        ]
      },
      floodDefense: {
        icon: '🏘️',
        title: 'Flood Defense',
        subtitle: 'Protect the villages from rising water',
        description: 'Aggressive springs threaten villages in the valleys. Build barriers and redirect floods to keep settlements safe!',
        objectives: [
          'Villages spawn in low ground - protect them!',
          'Springs appear on high ground and flood downhill',
          'Flooded villages damage your health over time',
          'New villages and springs appear as you play'
        ]
      }
    };
    return modeInfo[modeType] || null;
  }

  showPregameModal(modeType) {
    const modal = document.getElementById('pregame-modal');
    const info = this.getModeInfo(modeType);

    if (!modal || !info) return;

    // Update modal content
    modal.querySelector('.mode-icon').textContent = info.icon;
    modal.querySelector('.mode-title').textContent = info.title;
    modal.querySelector('.mode-subtitle').textContent = info.subtitle;
    modal.querySelector('.mode-description').textContent = info.description;

    const objectivesList = modal.querySelector('.mode-objectives ul');
    objectivesList.innerHTML = info.objectives.map(obj => `<li>${obj}</li>`).join('');

    // Store pending mode
    this.pendingGameMode = modeType;

    // Show modal
    modal.classList.add('visible');
  }

  hidePregameModal() {
    const modal = document.getElementById('pregame-modal');
    if (modal) {
      modal.classList.remove('visible');
    }
  }

  setupPregameModal() {
    const modal = document.getElementById('pregame-modal');
    if (!modal) return;

    const startBtn = modal.querySelector('.start-btn');
    if (startBtn) {
      startBtn.addEventListener('click', () => {
        if (this.pendingGameMode) {
          this.hidePregameModal();
          // Now actually start the game mode
          const started = this.gameModeManager.startMode(this.pendingGameMode);
          if (!started) {
            console.log(`Mode "${this.pendingGameMode}" not yet implemented`);
          }
          this.pendingGameMode = null;
        }
      });
    }
  }

  setupGameModeCallbacks() {
    // When mode starts
    this.gameModeManager.onModeStart = (modeType) => {
      if (modeType !== GameModeType.SANDBOX) {
        // Reset game state to defaults
        this.resetGameState();
        this.showArcadeHud(true);
      }
    };

    // When mode ends
    this.gameModeManager.onModeEnd = (modeType, completed, finalScore, stats) => {
      if (modeType !== GameModeType.SANDBOX) {
        this.showGameOver(finalScore, stats);
      }
    };

    // Score updates
    this.gameModeManager.onScoreUpdate = (score, combo) => {
      this.updateArcadeHud({ score, combo });
    };
  }

  setupModeSelector() {
    const modeBtn = document.getElementById('btn-mode');
    const modeDropdown = document.getElementById('mode-dropdown');
    const modeButtons = document.querySelectorAll('.mode-btn');

    if (!modeBtn || !modeDropdown) return;

    // Load high scores for display
    this.updateHighScoreDisplay();

    // Toggle dropdown
    modeBtn.addEventListener('click', (e) => {
      e.stopPropagation();
      // Close quality dropdown if open
      const qualityDropdown = document.getElementById('quality-dropdown');
      if (qualityDropdown) qualityDropdown.classList.remove('visible');
      modeDropdown.classList.toggle('visible');
    });

    // Handle mode selection
    modeButtons.forEach(btn => {
      btn.addEventListener('click', (e) => {
        e.stopPropagation();
        const mode = btn.dataset.mode;

        // Update active state
        modeButtons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');

        // Update button text
        const modeName = btn.querySelector('.mode-name').textContent;
        modeBtn.textContent = modeName;

        // Close dropdown
        modeDropdown.classList.remove('visible');

        // Start or end mode
        if (mode === 'sandbox') {
          this.gameModeManager.endMode(false);
          this.showArcadeHud(false);
        } else {
          // Show pre-game modal instead of starting immediately
          this.showPregameModal(mode);
        }
      });
    });

    // Close dropdown when clicking elsewhere
    document.addEventListener('click', () => {
      modeDropdown.classList.remove('visible');
    });

    // Setup game buttons (restart and quit)
    const restartBtn = document.querySelector('#arcade-hud .restart-btn');
    if (restartBtn) {
      restartBtn.addEventListener('click', () => {
        this.restartCurrentMode();
      });
    }

    const quitBtn = document.querySelector('#arcade-hud .quit-btn');
    if (quitBtn) {
      quitBtn.addEventListener('click', () => {
        this.quitToSandbox();
      });
    }

    // Setup high score entry
    this.setupHighScoreEntry();
  }

  setupHighScoreEntry() {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    const inputs = hud.querySelectorAll('.initials-input .initial');
    const submitBtn = hud.querySelector('.submit-score-btn');

    // Auto-advance to next input on typing
    inputs.forEach((input, index) => {
      input.addEventListener('input', (e) => {
        // Only allow letters
        e.target.value = e.target.value.replace(/[^a-zA-Z]/g, '').toUpperCase();

        // Auto-advance to next input
        if (e.target.value.length === 1 && index < inputs.length - 1) {
          inputs[index + 1].focus();
        }
      });

      // Handle backspace to go to previous input
      input.addEventListener('keydown', (e) => {
        if (e.key === 'Backspace' && e.target.value === '' && index > 0) {
          inputs[index - 1].focus();
        }
        // Submit on Enter
        if (e.key === 'Enter') {
          submitBtn.click();
        }
      });
    });

    // Submit button
    submitBtn.addEventListener('click', () => {
      const initials = Array.from(inputs).map(i => i.value || 'A').join('');
      this.submitHighScore(initials);
    });
  }

  submitHighScore(initials) {
    const scoreManager = new ScoreManager();
    const modeType = this.gameModeManager.lastPlayedMode;
    const score = this.pendingHighScore;

    if (modeType && score > 0) {
      scoreManager.addHighScore(modeType, score, initials);
      this.updateHighScoreDisplay();
      this.showLeaderboard(modeType, score);
    }

    // Hide entry form, show game buttons
    const hud = document.getElementById('arcade-hud');
    if (hud) {
      hud.querySelector('.highscore-entry').classList.remove('visible');
      hud.querySelector('.game-buttons').style.display = 'flex';
    }
  }

  showLeaderboard(modeType, highlightScore = null) {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    const scoreManager = new ScoreManager();
    const leaderboard = scoreManager.getLeaderboard(modeType);
    const listEl = hud.querySelector('.leaderboard-list');

    listEl.innerHTML = leaderboard.map((entry, index) => {
      const isHighlight = entry.score === highlightScore;
      return `
        <li class="${isHighlight ? 'highlight' : ''}">
          <span class="rank">${index + 1}.</span>
          <span class="initials">${entry.initials}</span>
          <span class="score">${entry.score.toLocaleString()}</span>
        </li>
      `;
    }).join('');

    hud.querySelector('.leaderboard').classList.add('visible');
  }

  updateHighScoreDisplay() {
    // Create temporary score manager to read high scores
    const scoreManager = new ScoreManager();
    const modes = ['flowRush', 'cascade', 'floodDefense'];

    modes.forEach(mode => {
      const el = document.getElementById(`highscore-${mode}`);
      if (el) {
        const score = scoreManager.getHighScore(mode);
        el.textContent = score > 0 ? `Best: ${score}` : '';
      }
    });
  }

  showArcadeHud(visible) {
    const hud = document.getElementById('arcade-hud');
    if (hud) {
      hud.classList.toggle('visible', visible);
      // Reset HUD state
      if (visible) {
        hud.querySelector('.game-over').style.display = 'none';
        hud.querySelector('.final-score').style.display = 'none';
        hud.querySelector('.new-high-score').style.display = 'none';
        hud.querySelector('.game-buttons').style.display = 'none';
        hud.querySelector('.highscore-entry').classList.remove('visible');
        hud.querySelector('.leaderboard').classList.remove('visible');
        hud.querySelector('.score').textContent = '0';
        hud.querySelector('.combo').textContent = 'x1';
        hud.querySelector('.combo-bar-fill').style.width = '0%';
        hud.querySelector('.timer').textContent = '2:00';
        hud.querySelector('.timer').classList.remove('urgent');
        hud.querySelector('.lives').innerHTML = '';
        this.pendingHighScore = null;
      }
    }
  }

  updateArcadeHud(info) {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    if (info.score !== undefined) {
      hud.querySelector('.score').textContent = info.score;
    }
    if (info.combo !== undefined) {
      hud.querySelector('.combo').textContent = `x${info.combo}`;
    }
    if (info.comboTimer !== undefined) {
      hud.querySelector('.combo-bar-fill').style.width = `${info.comboTimer * 100}%`;
    }
    if (info.quarterHearts !== undefined) {
      // Display quarter hearts (Zelda-style)
      const livesEl = hud.querySelector('.lives');
      livesEl.innerHTML = this.renderQuarterHearts(info.quarterHearts, info.maxQuarterHearts);
    } else if (info.lives !== undefined) {
      // Fallback: Display lives as full hearts
      const livesEl = hud.querySelector('.lives');
      livesEl.textContent = Array(info.lives).fill('\u2764').join(' ');
    }
    if (info.timeRemaining !== undefined) {
      const timerEl = hud.querySelector('.timer');
      const minutes = Math.floor(info.timeRemaining / 60);
      const seconds = Math.floor(info.timeRemaining % 60);
      timerEl.textContent = `${minutes}:${seconds.toString().padStart(2, '0')}`;
      // Add urgent class when time is low
      timerEl.classList.toggle('urgent', info.timeRemaining < 30);
    }
  }

  showGameOver(finalScore, stats) {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    const gameOverEl = hud.querySelector('.game-over');
    // Show "TIME'S UP" for time-based modes that completed successfully
    if (this.gameModeManager.currentMode && this.gameModeManager.currentMode.victory) {
      gameOverEl.textContent = "TIME'S UP!";
      gameOverEl.style.color = '#88ccff';
    } else {
      gameOverEl.textContent = 'GAME OVER';
      gameOverEl.style.color = '#ff6666';
    }
    gameOverEl.style.display = 'block';

    hud.querySelector('.final-score').style.display = 'block';
    hud.querySelector('.final-score').textContent = `Final Score: ${finalScore}`;

    // Check for top 5 high score
    const scoreManager = new ScoreManager();
    const modeType = this.gameModeManager.lastPlayedMode || this.gameModeManager.currentModeType;
    const isTopScore = scoreManager.isTopScore(modeType, finalScore);

    if (isTopScore) {
      // Show high score entry form
      hud.querySelector('.new-high-score').style.display = 'block';
      hud.querySelector('.highscore-entry').classList.add('visible');
      hud.querySelector('.game-buttons').style.display = 'none';

      // Store pending score for submission
      this.pendingHighScore = finalScore;

      // Clear and focus first input
      const inputs = hud.querySelectorAll('.initials-input .initial');
      inputs.forEach(i => i.value = '');
      inputs[0].focus();

      // Update rank text
      const rank = scoreManager.getRankForScore(modeType, finalScore);
      hud.querySelector('.rank-text').textContent = `#${rank} - Enter your initials:`;
    } else {
      // Just show leaderboard and game buttons
      hud.querySelector('.game-buttons').style.display = 'flex';
      if (scoreManager.getLeaderboard(modeType).length > 0) {
        this.showLeaderboard(modeType);
      }
    }
  }

  restartCurrentMode() {
    // Use lastPlayedMode since currentModeType resets to SANDBOX on game end
    const modeToRestart = this.gameModeManager.lastPlayedMode;
    if (modeToRestart) {
      // Hide game over UI first
      this.hideGameOverUI();
      // Show pre-game modal for fresh start
      this.showPregameModal(modeToRestart);
    }
  }

  hideGameOverUI() {
    const hud = document.getElementById('arcade-hud');
    if (!hud) return;

    hud.querySelector('.game-over').style.display = 'none';
    hud.querySelector('.final-score').style.display = 'none';
    hud.querySelector('.new-high-score').style.display = 'none';
    hud.querySelector('.highscore-entry').classList.remove('visible');
    hud.querySelector('.leaderboard').classList.remove('visible');
    hud.querySelector('.game-buttons').style.display = 'none';
    hud.classList.remove('visible');
  }

  quitToSandbox() {
    // Hide arcade HUD and return to sandbox
    this.showArcadeHud(false);
    this.gameModeManager.endMode(false);

    // Update mode selector button to show Sandbox
    const modeBtn = document.getElementById('btn-mode');
    if (modeBtn) {
      modeBtn.textContent = 'Sandbox';
    }

    // Update active state in dropdown
    const modeButtons = document.querySelectorAll('.mode-btn');
    modeButtons.forEach(btn => {
      btn.classList.toggle('active', btn.dataset.mode === 'sandbox');
    });
  }

  resetGameState() {
    // Reset time controls
    this.timeScale = 1.0;
    this.paused = false;

    // Reset brush settings to defaults
    this.terrain.brushRadius = 8;  // DEFAULT_BRUSH_RADIUS
    this.terrain.brushStrength = 2; // DEFAULT_BRUSH_STRENGTH

    // Reset to raise tool
    this.controls.currentTool = 'raise';

    // Update UI to reflect reset state
    this.controls.updateToolDisplay();
  }

  renderQuarterHearts(quarterHearts, maxQuarterHearts) {
    const totalHearts = Math.ceil(maxQuarterHearts / 4);
    let html = '';

    for (let i = 0; i < totalHearts; i++) {
      const quartersForThisHeart = Math.max(0, Math.min(4, quarterHearts - i * 4));

      if (quartersForThisHeart === 4) {
        // Full heart
        html += '<span class="heart full">\u2764</span>';
      } else if (quartersForThisHeart === 3) {
        // 3/4 heart
        html += '<span class="heart three-quarter">\u2764</span>';
      } else if (quartersForThisHeart === 2) {
        // Half heart
        html += '<span class="heart half">\u2764</span>';
      } else if (quartersForThisHeart === 1) {
        // 1/4 heart
        html += '<span class="heart quarter">\u2764</span>';
      } else {
        // Empty heart
        html += '<span class="heart empty">\u2661</span>';
      }
    }

    return html;
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

    // Update game mode (if playing)
    this.gameModeManager.update(dt);

    // Update arcade HUD with current mode info
    if (this.gameModeManager.isPlaying && this.gameModeManager.currentMode) {
      const info = this.gameModeManager.currentMode.getDisplayInfo();
      this.updateArcadeHud(info);
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
