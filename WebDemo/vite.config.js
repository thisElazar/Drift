import { defineConfig } from 'vite';
import { resolve } from 'path';

// Check which build we're doing
const buildTarget = process.env.BUILD_TARGET || 'desktop';

const configs = {
  desktop: {
    base: './',
    build: {
      outDir: 'dist',
      emptyOutDir: true,
      rollupOptions: {
        input: resolve(__dirname, 'index.html'),
        output: {
          entryFileNames: 'assets/drift.js',
          chunkFileNames: 'assets/[name].js',
          assetFileNames: 'assets/[name].[ext]',
        },
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
        output: {
          entryFileNames: 'assets/drift-mobile.js',
          chunkFileNames: 'assets/[name].js',
          assetFileNames: 'assets/[name].[ext]',
        },
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
