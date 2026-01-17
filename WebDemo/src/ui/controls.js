import * as THREE from 'three';
import { GRID_WIDTH, GRID_HEIGHT, WORLD_SCALE } from '../simulation/constants.js';

export const Tool = {
  NONE: 'none',
  RAISE: 'raise',
  LOWER: 'lower',
  WATER: 'water',
  SPRING: 'spring',
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

    // Configure orbit controls for right-click rotation, middle-click pan
    const orbitControls = this.scene.controls;
    orbitControls.enabled = true;
    orbitControls.mouseButtons = {
      LEFT: null,           // We handle left-click ourselves
      MIDDLE: THREE.MOUSE.PAN,
      RIGHT: THREE.MOUSE.ROTATE,
    };
    orbitControls.enableZoom = true;  // Scroll wheel zoom

    // Bind event handlers
    this.onMouseDown = this.onMouseDown.bind(this);
    this.onMouseUp = this.onMouseUp.bind(this);
    this.onMouseMove = this.onMouseMove.bind(this);
    this.onContextMenu = this.onContextMenu.bind(this);
    this.onKeyDown = this.onKeyDown.bind(this);

    this.setupEventListeners();
  }

  setupEventListeners() {
    const canvas = this.scene.canvas;

    canvas.addEventListener('mousedown', this.onMouseDown);
    canvas.addEventListener('mouseup', this.onMouseUp);
    canvas.addEventListener('mousemove', this.onMouseMove);
    canvas.addEventListener('contextmenu', this.onContextMenu);
    window.addEventListener('keydown', this.onKeyDown);
  }

  onContextMenu(event) {
    event.preventDefault(); // Prevent right-click menu
  }

  onKeyDown(event) {
    switch (event.key.toLowerCase()) {
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
      case 'r':
        this.terrain.reset();
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
    }
  }

  updateToolDisplay() {
    const info = document.getElementById('info');
    if (info) {
      info.innerHTML = `
        <h1>Drift Demo</h1>
        <p>Tool: <strong>${this.currentTool}</strong> | Size: ${this.terrain.brushRadius} | Strength: ${this.terrain.brushStrength.toFixed(1)}</p>
        <p style="font-size: 12px; opacity: 0.7; margin-top: 5px;">
          Left-click: Use tool | Right-drag: Rotate | Scroll: Zoom<br>
          1: Raise | 2: Lower | 3: Water | 4: Spring<br>
          [ / ]: Brush size | - / +: Strength | R: Reset terrain
        </p>
      `;
    }
  }

  onMouseDown(event) {
    // Only handle left-click for tools
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
    // Only apply tool on left-button drag
    if (this.isEditing && (event.buttons & 1)) {
      this.applyToolAtMouse(event);
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
      }
    }
  }

  // Callbacks for water system (set from main.js)
  onAddWater = null;
  onAddSpring = null;
}
