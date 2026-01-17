import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { TERRAIN_WIDTH, TERRAIN_HEIGHT, MAX_HEIGHT } from '../simulation/constants.js';

export class Scene {
  constructor(container) {
    this.container = container;

    // Create renderer
    this.renderer = new THREE.WebGLRenderer({ antialias: true });
    this.renderer.setSize(window.innerWidth, window.innerHeight);
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
    this.renderer.setClearColor(0x1a1a2e);
    container.appendChild(this.renderer.domElement);

    // Create scene
    this.scene = new THREE.Scene();
    this.scene.fog = new THREE.Fog(0x1a1a2e, TERRAIN_WIDTH, TERRAIN_WIDTH * 2);

    // Create camera
    const aspect = window.innerWidth / window.innerHeight;
    this.camera = new THREE.PerspectiveCamera(60, aspect, 1, TERRAIN_WIDTH * 4);
    this.camera.position.set(
      TERRAIN_WIDTH * 0.6,
      MAX_HEIGHT * 2,
      TERRAIN_HEIGHT * 0.6
    );

    // Orbit controls
    this.controls = new OrbitControls(this.camera, this.renderer.domElement);
    this.controls.enableDamping = true;
    this.controls.dampingFactor = 0.08;
    this.controls.rotateSpeed = 0.8;
    this.controls.panSpeed = 0.8;
    this.controls.zoomSpeed = 1.2;
    this.controls.target.set(0, 0, 0);
    this.controls.minPolarAngle = 0.1;  // Don't go exactly vertical
    this.controls.maxPolarAngle = Math.PI * 0.85; // Allow low angles
    this.controls.minDistance = 50;
    this.controls.maxDistance = TERRAIN_WIDTH * 3;

    // Lighting
    this.setupLighting();

    // Handle resize
    window.addEventListener('resize', () => this.onResize());
  }

  setupLighting() {
    // Ambient light
    const ambient = new THREE.AmbientLight(0x404060, 0.5);
    this.scene.add(ambient);

    // Directional sun light
    const sun = new THREE.DirectionalLight(0xffffee, 1.0);
    sun.position.set(TERRAIN_WIDTH * 0.5, MAX_HEIGHT * 3, TERRAIN_HEIGHT * 0.3);
    sun.castShadow = false; // Disabled for performance
    this.scene.add(sun);

    // Hemisphere light for natural sky/ground coloring
    const hemi = new THREE.HemisphereLight(0x87ceeb, 0x3d5c3d, 0.4);
    this.scene.add(hemi);
  }

  onResize() {
    const width = window.innerWidth;
    const height = window.innerHeight;

    this.camera.aspect = width / height;
    this.camera.updateProjectionMatrix();

    this.renderer.setSize(width, height);
  }

  add(object) {
    this.scene.add(object);
  }

  remove(object) {
    this.scene.remove(object);
  }

  render() {
    this.controls.update();
    this.renderer.render(this.scene, this.camera);
  }

  // Get the canvas for event listeners
  get canvas() {
    return this.renderer.domElement;
  }
}
