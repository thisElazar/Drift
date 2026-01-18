import { defineConfig } from 'vite';
import { resolve } from 'path';

// Check which build we're doing
const buildTarget = process.env.BUILD_TARGET || 'desktop';

const configs = {
  desktop: {
    base: '/drift/',
    build: {
      outDir: 'dist',
      emptyOutDir: true,
      rollupOptions: {
        input: resolve(__dirname, 'index.html'),
      },
    },
  },
  mobile: {
    base: './',
    build: {
      outDir: 'dist-mobile',
      emptyOutDir: true,
      rollupOptions: {
        input: resolve(__dirname, 'mobile.html'),
      },
    },
  },
};

const targetConfig = configs[buildTarget];

export default defineConfig({
  root: '.',
  base: targetConfig.base,
  publicDir: 'assets',
  build: targetConfig.build,
  server: {
    open: true,
  },
});
