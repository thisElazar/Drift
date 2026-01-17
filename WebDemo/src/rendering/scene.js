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
    this.renderer.toneMapping = THREE.ACESFilmicToneMapping;
    this.renderer.toneMappingExposure = 1.1;
    container.appendChild(this.renderer.domElement);

    // Create scene
    this.scene = new THREE.Scene();

    // Gradient sky background
    this.setupSkyBackground();

    // Atmospheric fog matching sky
    this.scene.fog = new THREE.FogExp2(0x88aabb, 0.00035);

    // Create camera
    const aspect = window.innerWidth / window.innerHeight;
    this.camera = new THREE.PerspectiveCamera(55, aspect, 1, TERRAIN_WIDTH * 5);
    this.camera.position.set(
      TERRAIN_WIDTH * 0.5,
      MAX_HEIGHT * 2.5,
      TERRAIN_HEIGHT * 0.5
    );

    // Orbit controls
    this.controls = new OrbitControls(this.camera, this.renderer.domElement);
    this.controls.enableDamping = true;
    this.controls.dampingFactor = 0.08;
    this.controls.rotateSpeed = 0.8;
    this.controls.panSpeed = 0.8;
    this.controls.zoomSpeed = 1.2;
    this.controls.target.set(0, 0, 0);
    this.controls.minPolarAngle = 0.1;
    this.controls.maxPolarAngle = Math.PI * 0.85;
    this.controls.minDistance = 80;
    this.controls.maxDistance = TERRAIN_WIDTH * 2.5;

    // Lighting
    this.setupLighting();

    // Sun animation
    this.sunAngle = 0;
    this.sunSpeed = 0.0375; // Radians per second (slow, speeds up with time scale)

    // Handle resize
    window.addEventListener('resize', () => this.onResize());
  }

  setupSkyBackground() {
    // Create gradient sky using a large sphere
    const skyGeo = new THREE.SphereGeometry(TERRAIN_WIDTH * 4, 32, 32);
    const skyMat = new THREE.ShaderMaterial({
      uniforms: {
        topColor: { value: new THREE.Color(0x66bbff) },    // Bright sky blue at top
        middleColor: { value: new THREE.Color(0xaaccee) }, // Lighter blue at horizon
        bottomColor: { value: new THREE.Color(0x667788) }, // Lighter blue-gray below
        offset: { value: 20 },
        exponent: { value: 0.6 }
      },
      vertexShader: `
        varying vec3 vWorldPosition;
        void main() {
          vec4 worldPosition = modelMatrix * vec4(position, 1.0);
          vWorldPosition = worldPosition.xyz;
          gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
        }
      `,
      fragmentShader: `
        uniform vec3 topColor;
        uniform vec3 middleColor;
        uniform vec3 bottomColor;
        uniform float offset;
        uniform float exponent;
        varying vec3 vWorldPosition;
        void main() {
          float h = normalize(vWorldPosition + offset).y;
          if (h > 0.0) {
            gl_FragColor = vec4(mix(middleColor, topColor, pow(h, exponent)), 1.0);
          } else {
            gl_FragColor = vec4(mix(middleColor, bottomColor, pow(-h, 0.5)), 1.0);
          }
        }
      `,
      side: THREE.BackSide
    });
    const sky = new THREE.Mesh(skyGeo, skyMat);
    this.scene.add(sky);
  }

  setupLighting() {
    // Hemisphere light - sky blue from above, warm ground bounce from below
    const hemi = new THREE.HemisphereLight(0x87ceeb, 0x908060, 1.0);
    this.scene.add(hemi);

    // Main sun light - warm directional
    this.sun = new THREE.DirectionalLight(0xfff4e5, 1.6);
    this.sun.position.set(TERRAIN_WIDTH * 0.4, MAX_HEIGHT * 4, TERRAIN_HEIGHT * 0.2);
    this.scene.add(this.sun);

    // Visible sun disc
    const sunGeo = new THREE.CircleGeometry(TERRAIN_WIDTH * 0.08, 32);
    const sunMat = new THREE.MeshBasicMaterial({
      color: 0xffffdd,
      fog: false,
    });
    this.sunDisc = new THREE.Mesh(sunGeo, sunMat);
    this.sunDisc.position.copy(this.sun.position);
    this.scene.add(this.sunDisc);

    // Sun glow (larger, semi-transparent)
    const glowGeo = new THREE.CircleGeometry(TERRAIN_WIDTH * 0.15, 32);
    const glowMat = new THREE.MeshBasicMaterial({
      color: 0xffeeaa,
      transparent: true,
      opacity: 0.3,
      fog: false,
    });
    this.sunGlow = new THREE.Mesh(glowGeo, glowMat);
    this.sunGlow.position.copy(this.sun.position);
    this.scene.add(this.sunGlow);

    // Fill light from opposite side - cool and subtle
    const fill = new THREE.DirectionalLight(0xaabbcc, 0.5);
    fill.position.set(-TERRAIN_WIDTH * 0.3, MAX_HEIGHT * 2, -TERRAIN_HEIGHT * 0.4);
    this.scene.add(fill);

    // Subtle ambient to lift shadows
    const ambient = new THREE.AmbientLight(0x606070, 0.4);
    this.scene.add(ambient);
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

  update(dt) {
    // Animate sun in a circle around the terrain
    this.sunAngle += this.sunSpeed * dt;

    const radius = TERRAIN_WIDTH * 1.5;
    const height = MAX_HEIGHT * 4 + Math.sin(this.sunAngle * 0.5) * MAX_HEIGHT * 2;

    this.sun.position.x = Math.cos(this.sunAngle) * radius;
    this.sun.position.z = Math.sin(this.sunAngle) * radius;
    this.sun.position.y = height;

    // Sun always looks at center
    this.sun.target.position.set(0, 0, 0);

    // Move sun disc and glow to match light position (scaled out for sky)
    const discRadius = TERRAIN_WIDTH * 2.5;
    const discHeight = height * 1.5;
    this.sunDisc.position.x = Math.cos(this.sunAngle) * discRadius;
    this.sunDisc.position.z = Math.sin(this.sunAngle) * discRadius;
    this.sunDisc.position.y = discHeight;

    this.sunGlow.position.copy(this.sunDisc.position);

    // Make sun disc and glow face the camera
    this.sunDisc.lookAt(this.camera.position);
    this.sunGlow.lookAt(this.camera.position);
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
