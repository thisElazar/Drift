import * as THREE from 'three';
import { GRID_WIDTH, GRID_HEIGHT, WORLD_SCALE } from '../simulation/constants.js';

export const Tool = {
  NONE: 'none',
  RAISE: 'raise',
  LOWER: 'lower',
  WATER: 'water',
  SPRING: 'spring',
  REMOVE_SPRING: 'remove spring',
};

export class TerrainControls {
  constructor(scene, terrain, terrainMesh) {
    this.scene = scene;
    this.terrain = terrain;
    this.terrainMesh = terrainMesh;

    this.currentTool = Tool.RAISE;
    this.isEditing = false;
    this.raycaster = new THREE.Raycaster();
    this.mouse = new THREE.Vector2();

    // Stats
    this.fps = 0;
    this.waterVolume = 0;
    this.springCount = 0;

    // Brush cursor preview
    this.brushCursor = this.createBrushCursor();
    this.scene.add(this.brushCursor);

    // Configure OrbitControls
    this.orbitControls = this.scene.controls;
    this.orbitControls.enabled = true;
    this.orbitControls.mouseButtons = {
      LEFT: null,                 // We handle left-click for tools
      MIDDLE: THREE.MOUSE.DOLLY,  // Scroll/middle for zoom
      RIGHT: THREE.MOUSE.ROTATE,  // Right-drag to rotate
    };
    this.orbitControls.enablePan = true;
    this.orbitControls.enableZoom = true;
    this.orbitControls.enableDamping = true;
    this.orbitControls.screenSpacePanning = true;  // Pan in screen space

    // WASD + QE for panning
    this.keys = { w: false, a: false, s: false, d: false, q: false, e: false };
    this.panSpeed = 150;

    // Bind event handlers
    this.onMouseDown = this.onMouseDown.bind(this);
    this.onMouseUp = this.onMouseUp.bind(this);
    this.onMouseMove = this.onMouseMove.bind(this);
    this.onContextMenu = this.onContextMenu.bind(this);
    this.onKeyDown = this.onKeyDown.bind(this);
    this.onKeyUp = this.onKeyUp.bind(this);

    this.setupEventListeners();
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
    const radius = this.terrain.brushRadius * WORLD_SCALE;
    this.brushCursor.scale.set(radius, radius, radius);
    this.brushCursor.position.set(x, y + 1, z);
    this.brushCursor.visible = true;
  }

  hideBrushCursor() {
    this.brushCursor.visible = false;
  }

  setupEventListeners() {
    const canvas = this.scene.canvas;

    canvas.addEventListener('mousedown', this.onMouseDown);
    canvas.addEventListener('mouseup', this.onMouseUp);
    canvas.addEventListener('mousemove', this.onMouseMove);
    canvas.addEventListener('mouseleave', () => this.hideBrushCursor());
    canvas.addEventListener('contextmenu', this.onContextMenu);
    window.addEventListener('keydown', this.onKeyDown);
    window.addEventListener('keyup', this.onKeyUp);
  }

  onContextMenu(event) {
    event.preventDefault(); // Prevent right-click menu
  }

  onKeyDown(event) {
    const key = event.key.toLowerCase();

    // WASD + QE movement
    if (key in this.keys) {
      this.keys[key] = true;
      return;
    }

    switch (key) {
      case '1':
        this.currentTool = Tool.RAISE;
        this.updateToolDisplay();
        break;
      case '2':
        this.currentTool = Tool.LOWER;
        this.updateToolDisplay();
        break;
      case '3':
        this.currentTool = Tool.WATER;
        this.updateToolDisplay();
        break;
      case '4':
        this.currentTool = Tool.SPRING;
        this.updateToolDisplay();
        break;
      case '5':
        this.currentTool = Tool.REMOVE_SPRING;
        this.updateToolDisplay();
        break;
      case 'r':
        this.terrain.reset();
        break;
      case 'c':
        if (this.onClearWater) this.onClearWater();
        break;
      // Brush size: [ and ]
      case '[':
        this.terrain.brushRadius = Math.max(1, this.terrain.brushRadius - 1);
        this.updateToolDisplay();
        break;
      case ']':
        this.terrain.brushRadius = Math.min(30, this.terrain.brushRadius + 1);
        this.updateToolDisplay();
        break;
      // Brush strength: - and + (= key)
      case '-':
        this.terrain.brushStrength = Math.max(0.5, this.terrain.brushStrength - 0.5);
        this.updateToolDisplay();
        break;
      case '=':
      case '+':
        this.terrain.brushStrength = Math.min(20, this.terrain.brushStrength + 0.5);
        this.updateToolDisplay();
        break;
      // Time controls
      case ',':
      case '<':
        if (this.onTimeScaleChange) this.onTimeScaleChange(-0.25);
        break;
      case '.':
      case '>':
        if (this.onTimeScaleChange) this.onTimeScaleChange(0.25);
        break;
      case 'p':
        if (this.onTogglePause) this.onTogglePause();
        break;
    }
  }

  onKeyUp(event) {
    const key = event.key.toLowerCase();
    if (key in this.keys) {
      this.keys[key] = false;
    }
  }

  updateToolDisplay() {
    const panel = document.getElementById('controls-panel');
    if (panel) {
      const toolName = panel.querySelector('.tool-name');
      const brushInfo = panel.querySelector('.brush-info');
      if (toolName) toolName.textContent = this.currentTool;
      if (brushInfo) brushInfo.textContent = `Size: ${this.terrain.brushRadius} | Strength: ${this.terrain.brushStrength.toFixed(1)}`;
    }
  }

  updateStats(fps, waterVolume, springCount, timeScale = 1, paused = false) {
    this.fps = Math.round(fps);
    this.waterVolume = waterVolume;
    this.springCount = springCount;

    const panel = document.getElementById('controls-panel');
    if (panel) {
      const stats = panel.querySelector('.stats');
      if (stats) {
        let timeInfo = paused ? 'PAUSED' : `${timeScale.toFixed(2)}x`;
        stats.textContent = `${this.fps} FPS | ${timeInfo} | Water: ${this.waterVolume.toFixed(0)} | Springs: ${this.springCount}`;
      }
    }
  }

  onMouseDown(event) {
    // Left-click: use tools
    if (event.button === 0) {
      this.isEditing = true;
      this.applyToolAtMouse(event);
    }
  }

  onMouseUp(event) {
    if (event.button === 0) {
      this.isEditing = false;
    }
  }

  onMouseMove(event) {
    // Update brush cursor position
    const rect = this.scene.canvas.getBoundingClientRect();
    this.mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    this.mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

    this.raycaster.setFromCamera(this.mouse, this.scene.camera);
    const intersects = this.raycaster.intersectObject(this.terrainMesh.mesh);

    if (intersects.length > 0) {
      const point = intersects[0].point;
      this.updateBrushCursor(point.x, point.y, point.z);
    } else {
      this.hideBrushCursor();
    }

    // Apply tool on left-button drag
    if (this.isEditing && (event.buttons & 1)) {
      this.applyToolAtMouse(event);
    }
  }

  // Call this from main.js each frame
  updateCamera(dt) {
    // Get camera's right and forward vectors in world space (horizontal only)
    const camera = this.scene.camera;
    const target = this.orbitControls.target;

    // Get camera direction projected to XZ plane
    const cameraDir = new THREE.Vector3();
    camera.getWorldDirection(cameraDir);
    cameraDir.y = 0;
    cameraDir.normalize();

    // Right vector
    const right = new THREE.Vector3();
    right.crossVectors(cameraDir, new THREE.Vector3(0, 1, 0)).normalize();

    const speed = this.panSpeed * dt;

    // WASD pans by moving both camera and target
    if (this.keys.w) {
      camera.position.add(cameraDir.clone().multiplyScalar(speed));
      target.add(cameraDir.clone().multiplyScalar(speed));
    }
    if (this.keys.s) {
      camera.position.add(cameraDir.clone().multiplyScalar(-speed));
      target.add(cameraDir.clone().multiplyScalar(-speed));
    }
    if (this.keys.d) {
      camera.position.add(right.clone().multiplyScalar(speed));
      target.add(right.clone().multiplyScalar(speed));
    }
    if (this.keys.a) {
      camera.position.add(right.clone().multiplyScalar(-speed));
      target.add(right.clone().multiplyScalar(-speed));
    }

    // QE moves up/down
    if (this.keys.e) {
      camera.position.y += speed;
      target.y += speed;
    }
    if (this.keys.q) {
      camera.position.y -= speed;
      target.y -= speed;
    }
  }

  applyToolAtMouse(event) {
    // Convert mouse to normalized device coordinates
    const rect = this.scene.canvas.getBoundingClientRect();
    this.mouse.x = ((event.clientX - rect.left) / rect.width) * 2 - 1;
    this.mouse.y = -((event.clientY - rect.top) / rect.height) * 2 + 1;

    // Raycast to terrain
    this.raycaster.setFromCamera(this.mouse, this.scene.camera);
    const intersects = this.raycaster.intersectObject(this.terrainMesh.mesh);

    if (intersects.length > 0) {
      const point = intersects[0].point;

      // Convert world position to grid coordinates
      const gridX = (point.x / WORLD_SCALE) + GRID_WIDTH / 2;
      const gridY = (point.z / WORLD_SCALE) + GRID_HEIGHT / 2;

      switch (this.currentTool) {
        case Tool.RAISE:
          this.terrain.raise(gridX, gridY);
          break;
        case Tool.LOWER:
          this.terrain.lower(gridX, gridY);
          break;
        case Tool.WATER:
          if (this.onAddWater) {
            this.onAddWater(gridX, gridY);
          }
          break;
        case Tool.SPRING:
          if (this.onAddSpring) {
            this.onAddSpring(gridX, gridY);
          }
          break;
        case Tool.REMOVE_SPRING:
          if (this.onRemoveSpring) {
            this.onRemoveSpring(gridX, gridY);
          }
          break;
      }
    }
  }

  // Callbacks for water system (set from main.js)
  onAddWater = null;
  onAddSpring = null;
  onRemoveSpring = null;
  onClearWater = null;

  // Callbacks for time control (set from main.js)
  onTimeScaleChange = null;
  onTogglePause = null;
}
