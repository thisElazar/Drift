import * as THREE from 'three';
import { getWorldScale, getGridWidth, getGridHeight } from '../simulation/constants.js';

export const Tool = {
  NONE: 'none',
  RAISE: 'raise',
  LOWER: 'lower',
  WATER: 'water',
  SPRING: 'spring',
};

export class MobileControls {
  constructor(scene, terrain, terrainMesh) {
    this.scene = scene;
    this.terrain = terrain;
    this.terrainMesh = terrainMesh;
    this.worldScale = getWorldScale();
    this.gridWidth = getGridWidth();
    this.gridHeight = getGridHeight();

    this.currentTool = Tool.RAISE;
    this.raycaster = new THREE.Raycaster();
    this.touchPoint = new THREE.Vector2();

    // Touch state
    this.activeTouches = new Map();
    this.lastTouchDist = 0;
    this.lastTouchCenter = { x: 0, y: 0 };
    this.lastTouchAngle = 0;
    this.isSingleTouch = false;
    this.toolAppliedThisGesture = false;

    // Brush cursor preview
    this.brushCursor = this.createBrushCursor();
    this.scene.add(this.brushCursor);

    // Camera reference
    this.camera = this.scene.camera;
    this.orbitTarget = this.scene.controls.target;

    // Disable OrbitControls - we handle everything
    this.scene.controls.enabled = false;

    // Bind handlers
    this.onTouchStart = this.onTouchStart.bind(this);
    this.onTouchMove = this.onTouchMove.bind(this);
    this.onTouchEnd = this.onTouchEnd.bind(this);

    this.setupTouchListeners();
    this.setupUIListeners();
  }

  createBrushCursor() {
    const geometry = new THREE.RingGeometry(0.9, 1, 32);
    geometry.rotateX(-Math.PI / 2);
    const material = new THREE.MeshBasicMaterial({
      color: 0xffffff,
      transparent: true,
      opacity: 0.6,
      side: THREE.DoubleSide,
      depthTest: false,
    });
    const cursor = new THREE.Mesh(geometry, material);
    cursor.renderOrder = 999;
    cursor.visible = false;
    return cursor;
  }

  updateBrushCursor(x, y, z) {
    const radius = this.terrain.brushRadius * this.worldScale;
    this.brushCursor.scale.set(radius, radius, radius);
    this.brushCursor.position.set(x, y + 1, z);
    this.brushCursor.visible = true;
  }

  hideBrushCursor() {
    this.brushCursor.visible = false;
  }

  setupTouchListeners() {
    const canvas = this.scene.canvas;
    canvas.addEventListener('touchstart', this.onTouchStart, { passive: false });
    canvas.addEventListener('touchmove', this.onTouchMove, { passive: false });
    canvas.addEventListener('touchend', this.onTouchEnd, { passive: false });
    canvas.addEventListener('touchcancel', this.onTouchEnd, { passive: false });
  }

  setupUIListeners() {
    // Tool buttons
    const toolButtons = document.querySelectorAll('.tool-btn');
    toolButtons.forEach(btn => {
      btn.addEventListener('touchstart', (e) => {
        e.stopPropagation();
        const tool = btn.dataset.tool;
        this.currentTool = tool;
        toolButtons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
      });
      // Also support click for non-touch devices
      btn.addEventListener('click', (e) => {
        e.stopPropagation();
        const tool = btn.dataset.tool;
        this.currentTool = tool;
        toolButtons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
      });
    });

    // New Map button (opens preset dropdown)
    const newMapBtn = document.getElementById('btn-new-map');
    const presetDropdown = document.getElementById('preset-dropdown');
    if (newMapBtn && presetDropdown) {
      const toggleDropdown = (e) => {
        e.preventDefault();
        e.stopPropagation();
        presetDropdown.classList.toggle('visible');
        // Hide quality dropdown
        document.getElementById('quality-dropdown')?.classList.remove('visible');
      };
      newMapBtn.addEventListener('touchend', toggleDropdown);
      newMapBtn.addEventListener('click', (e) => {
        if (!e.sourceCapabilities?.firesTouchEvents) toggleDropdown(e);
      });

      // Close dropdown when tapping elsewhere
      document.addEventListener('touchend', (e) => {
        setTimeout(() => {
          if (presetDropdown.classList.contains('visible')) {
            if (!presetDropdown.contains(e.target) && e.target !== newMapBtn) {
              presetDropdown.classList.remove('visible');
            }
          }
        }, 50);
      });
    }

    // Preset buttons
    const presetButtons = document.querySelectorAll('.preset-btn');
    presetButtons.forEach(btn => {
      const handlePreset = (e) => {
        e.preventDefault();
        e.stopPropagation();
        const preset = btn.dataset.preset;
        this.terrain.generate(preset);
        if (this.onTerrainReset) this.onTerrainReset();
        presetButtons.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        if (presetDropdown) presetDropdown.classList.remove('visible');
      };
      btn.addEventListener('touchend', handlePreset);
      btn.addEventListener('click', (e) => {
        if (!e.sourceCapabilities?.firesTouchEvents) handlePreset(e);
      });
    });

    // Reset button
    const resetBtn = document.getElementById('btn-reset');
    if (resetBtn) {
      const handleReset = (e) => {
        e.preventDefault();
        e.stopPropagation();
        this.terrain.reset();
        if (this.onTerrainReset) this.onTerrainReset();
      };
      resetBtn.addEventListener('touchend', handleReset);
      resetBtn.addEventListener('click', (e) => {
        if (!e.sourceCapabilities?.firesTouchEvents) handleReset(e);
      });
    }

    // Home button (reset camera view)
    const homeBtn = document.getElementById('btn-home');
    if (homeBtn) {
      const handleHome = (e) => {
        e.preventDefault();
        e.stopPropagation();
        this.scene.resetCamera();
      };
      homeBtn.addEventListener('touchend', handleHome);
      homeBtn.addEventListener('click', (e) => {
        if (!e.sourceCapabilities?.firesTouchEvents) handleHome(e);
      });
    }

    // Size sliders (tablet + phone versions)
    const sizeSlider = document.getElementById('size-slider');
    const sizeSliderPhone = document.getElementById('size-slider-phone');
    const sizeValue = document.getElementById('size-value');
    const sizeValuePhone = document.getElementById('size-value-phone');

    const updateSize = (value) => {
      this.terrain.brushRadius = parseInt(value);
      if (sizeValue) sizeValue.textContent = this.terrain.brushRadius;
      if (sizeValuePhone) sizeValuePhone.textContent = this.terrain.brushRadius;
      if (sizeSlider) sizeSlider.value = this.terrain.brushRadius;
      if (sizeSliderPhone) sizeSliderPhone.value = this.terrain.brushRadius;
    };

    [sizeSlider, sizeSliderPhone].forEach(slider => {
      if (slider) {
        slider.value = this.terrain.brushRadius;
        slider.addEventListener('input', (e) => {
          e.stopPropagation();
          updateSize(e.target.value);
        });
        slider.addEventListener('touchstart', (e) => e.stopPropagation());
        slider.addEventListener('touchmove', (e) => e.stopPropagation());
      }
    });
    if (sizeValue) sizeValue.textContent = this.terrain.brushRadius;
    if (sizeValuePhone) sizeValuePhone.textContent = this.terrain.brushRadius;

    // Strength sliders (tablet + phone versions)
    const strengthSlider = document.getElementById('strength-slider');
    const strengthSliderPhone = document.getElementById('strength-slider-phone');
    const strengthValue = document.getElementById('strength-value');
    const strengthValuePhone = document.getElementById('strength-value-phone');

    const updateStrength = (value) => {
      this.terrain.brushStrength = parseFloat(value);
      if (strengthValue) strengthValue.textContent = this.terrain.brushStrength.toFixed(1);
      if (strengthValuePhone) strengthValuePhone.textContent = this.terrain.brushStrength.toFixed(1);
      if (strengthSlider) strengthSlider.value = this.terrain.brushStrength;
      if (strengthSliderPhone) strengthSliderPhone.value = this.terrain.brushStrength;
    };

    [strengthSlider, strengthSliderPhone].forEach(slider => {
      if (slider) {
        slider.value = this.terrain.brushStrength;
        slider.addEventListener('input', (e) => {
          e.stopPropagation();
          updateStrength(e.target.value);
        });
        slider.addEventListener('touchstart', (e) => e.stopPropagation());
        slider.addEventListener('touchmove', (e) => e.stopPropagation());
      }
    });
    if (strengthValue) strengthValue.textContent = this.terrain.brushStrength.toFixed(1);
    if (strengthValuePhone) strengthValuePhone.textContent = this.terrain.brushStrength.toFixed(1);

    // Help button
    const helpBtn = document.getElementById('btn-help-top');
    const helpOverlay = document.getElementById('help-overlay');
    const closeHelp = document.getElementById('close-help');
    if (helpBtn && helpOverlay) {
      const showHelp = (e) => {
        e.preventDefault();
        e.stopPropagation();
        helpOverlay.classList.add('visible');
      };
      helpBtn.addEventListener('touchend', showHelp);
      helpBtn.addEventListener('click', (e) => {
        if (!e.sourceCapabilities?.firesTouchEvents) showHelp(e);
      });
    }
    if (closeHelp && helpOverlay) {
      const hideHelp = (e) => {
        e.preventDefault();
        e.stopPropagation();
        helpOverlay.classList.remove('visible');
      };
      closeHelp.addEventListener('touchend', hideHelp);
      closeHelp.addEventListener('click', (e) => {
        if (!e.sourceCapabilities?.firesTouchEvents) hideHelp(e);
      });
      helpOverlay.addEventListener('touchend', (e) => {
        if (e.target === helpOverlay) {
          helpOverlay.classList.remove('visible');
        }
      });
      helpOverlay.addEventListener('click', (e) => {
        if (e.target === helpOverlay) {
          helpOverlay.classList.remove('visible');
        }
      });
    }

    // Prevent default on all UI elements
    document.querySelectorAll('#top-bar, #bottom-bar, #preset-dropdown, #quality-dropdown, .side-slider').forEach(el => {
      el.addEventListener('touchstart', (e) => e.stopPropagation(), { passive: false });
      el.addEventListener('touchmove', (e) => e.stopPropagation(), { passive: false });
    });
  }

  updateStats(fps, waterVolume, springCount) {
    const stats = document.getElementById('stats');
    if (stats) {
      stats.textContent = `${Math.round(fps)} FPS | Water: ${waterVolume.toFixed(0)} | Springs: ${springCount}`;
    }
  }

  // ============ TOUCH HANDLERS ============

  getTouchDistance(t1, t2) {
    const dx = t1.clientX - t2.clientX;
    const dy = t1.clientY - t2.clientY;
    return Math.sqrt(dx * dx + dy * dy);
  }

  getTouchCenter(t1, t2) {
    return {
      x: (t1.clientX + t2.clientX) / 2,
      y: (t1.clientY + t2.clientY) / 2,
    };
  }

  getTouchAngle(t1, t2) {
    return Math.atan2(t2.clientY - t1.clientY, t2.clientX - t1.clientX);
  }

  onTouchStart(event) {
    event.preventDefault();

    // Update active touches
    for (const touch of event.changedTouches) {
      this.activeTouches.set(touch.identifier, {
        x: touch.clientX,
        y: touch.clientY,
      });
    }

    const touchCount = this.activeTouches.size;

    if (touchCount === 1) {
      this.isSingleTouch = true;
      this.toolAppliedThisGesture = false;
      const touch = event.changedTouches[0];
      this.applyToolAtTouch(touch.clientX, touch.clientY);
      this.toolAppliedThisGesture = true;
    } else if (touchCount === 2) {
      this.isSingleTouch = false;
      this.hideBrushCursor();

      // Setup two-finger gesture tracking
      const touches = Array.from(this.activeTouches.values());
      const t1 = { clientX: touches[0].x, clientY: touches[0].y };
      const t2 = { clientX: touches[1].x, clientY: touches[1].y };

      this.lastTouchDist = this.getTouchDistance(t1, t2);
      this.lastTouchCenter = this.getTouchCenter(t1, t2);
      this.lastTouchAngle = this.getTouchAngle(t1, t2);
    }
  }

  onTouchMove(event) {
    event.preventDefault();

    // Update active touches
    for (const touch of event.changedTouches) {
      if (this.activeTouches.has(touch.identifier)) {
        this.activeTouches.set(touch.identifier, {
          x: touch.clientX,
          y: touch.clientY,
        });
      }
    }

    const touchCount = this.activeTouches.size;

    if (touchCount === 1 && this.isSingleTouch) {
      // Single finger - apply tool (except spring which is tap-only)
      const touch = event.changedTouches[0];
      if (this.currentTool !== Tool.SPRING) {
        this.applyToolAtTouch(touch.clientX, touch.clientY);
      }
      this.updateBrushFromTouch(touch.clientX, touch.clientY);
    } else if (touchCount === 2) {
      // Two finger - camera control
      const touches = Array.from(this.activeTouches.values());
      const t1 = { clientX: touches[0].x, clientY: touches[0].y };
      const t2 = { clientX: touches[1].x, clientY: touches[1].y };

      const newDist = this.getTouchDistance(t1, t2);
      const newCenter = this.getTouchCenter(t1, t2);
      const newAngle = this.getTouchAngle(t1, t2);

      // Pinch zoom
      const distDelta = newDist - this.lastTouchDist;
      if (Math.abs(distDelta) > 0.5) {
        const zoomDir = new THREE.Vector3()
          .subVectors(this.orbitTarget, this.camera.position)
          .normalize();
        const zoomAmount = distDelta * 2.5;
        this.camera.position.add(zoomDir.clone().multiplyScalar(zoomAmount));
      }

      // Pan
      const panDeltaX = newCenter.x - this.lastTouchCenter.x;
      const panDeltaY = newCenter.y - this.lastTouchCenter.y;
      if (Math.abs(panDeltaX) > 0.5 || Math.abs(panDeltaY) > 0.5) {
        const right = new THREE.Vector3();
        const up = new THREE.Vector3(0, 1, 0);
        this.camera.getWorldDirection(right);
        right.crossVectors(up, right).normalize();

        const panSpeed = 2.5;
        const panX = right.clone().multiplyScalar(-panDeltaX * panSpeed);
        const panY = up.clone().multiplyScalar(panDeltaY * panSpeed);

        this.camera.position.add(panX).add(panY);
        this.orbitTarget.add(panX).add(panY);
      }

      // Rotate
      const angleDelta = newAngle - this.lastTouchAngle;
      if (Math.abs(angleDelta) > 0.005) {
        const offset = this.camera.position.clone().sub(this.orbitTarget);
        const cos = Math.cos(-angleDelta);
        const sin = Math.sin(-angleDelta);
        const newX = offset.x * cos - offset.z * sin;
        const newZ = offset.x * sin + offset.z * cos;
        offset.x = newX;
        offset.z = newZ;
        this.camera.position.copy(this.orbitTarget).add(offset);
        this.camera.lookAt(this.orbitTarget);
      }

      this.lastTouchDist = newDist;
      this.lastTouchCenter = newCenter;
      this.lastTouchAngle = newAngle;
    }
  }

  onTouchEnd(event) {
    event.preventDefault();

    // Remove ended touches
    for (const touch of event.changedTouches) {
      this.activeTouches.delete(touch.identifier);
    }

    if (this.activeTouches.size === 0) {
      this.isSingleTouch = false;
      this.hideBrushCursor();
    } else if (this.activeTouches.size === 1) {
      // Went from 2 to 1 finger - don't start tool, wait for new gesture
      this.isSingleTouch = false;
    }
  }

  applyToolAtTouch(clientX, clientY) {
    const rect = this.scene.canvas.getBoundingClientRect();
    this.touchPoint.x = ((clientX - rect.left) / rect.width) * 2 - 1;
    this.touchPoint.y = -((clientY - rect.top) / rect.height) * 2 + 1;

    this.raycaster.setFromCamera(this.touchPoint, this.camera);
    const intersects = this.raycaster.intersectObject(this.terrainMesh.mesh);

    if (intersects.length > 0) {
      const point = intersects[0].point;
      const gridX = (point.x / this.worldScale) + this.gridWidth / 2;
      const gridY = (point.z / this.worldScale) + this.gridHeight / 2;

      switch (this.currentTool) {
        case Tool.RAISE:
          this.terrain.raise(gridX, gridY);
          break;
        case Tool.LOWER:
          this.terrain.lower(gridX, gridY);
          break;
        case Tool.WATER:
          if (this.onAddWater) this.onAddWater(gridX, gridY);
          break;
        case Tool.SPRING:
          if (this.onAddSpring) this.onAddSpring(gridX, gridY);
          break;
      }

      this.updateBrushCursor(point.x, point.y, point.z);
    }
  }

  updateBrushFromTouch(clientX, clientY) {
    const rect = this.scene.canvas.getBoundingClientRect();
    this.touchPoint.x = ((clientX - rect.left) / rect.width) * 2 - 1;
    this.touchPoint.y = -((clientY - rect.top) / rect.height) * 2 + 1;

    this.raycaster.setFromCamera(this.touchPoint, this.camera);
    const intersects = this.raycaster.intersectObject(this.terrainMesh.mesh);

    if (intersects.length > 0) {
      const point = intersects[0].point;
      this.updateBrushCursor(point.x, point.y, point.z);
    }
  }

  // Callbacks (set from main-mobile.js)
  onAddWater = null;
  onAddSpring = null;
  onTerrainReset = null;
}
