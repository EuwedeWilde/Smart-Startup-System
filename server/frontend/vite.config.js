import { defineConfig } from 'vite'
import { svelte } from '@sveltejs/vite-plugin-svelte'

// https://vite.dev/config/
export default defineConfig({
  plugins: [svelte()],
  base: './', // Set the base path to the current directory
  server: {
    port: 3000, // Set the port to 3000
  },
  build: {
    outDir: 'dist', // Set the output directory to the parent directory
  },
})
