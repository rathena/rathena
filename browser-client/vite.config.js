import { defineConfig } from 'vite';
import path from 'path';
import { fileURLToPath } from 'url';
import uiCssHmrPlugin from './vite/csshotreload.plugin.js';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

const isDocker = process.env.RO_PROXY_TARGET === 'docker';
const webTarget = isDocker ? 'http://rathena-web:8888' : 'http://127.0.0.1:8888';  
const remoteClientTarget = isDocker ? 'http://remote-client-php:80' : 'http://127.0.0.1:8000';  
  
const _proxy = {  
	'/get': {  
		target: webTarget,  
		changeOrigin: true,  
		secure: false,  
		ws: false  
	},  
	'/emblem': {  
		target: webTarget,  
		changeOrigin: true,  
		secure: false,  
		ws: false  
	},  
	'/userconfig': {  
		target: webTarget,  
		changeOrigin: true,  
		secure: false,  
		ws: false  
	}  
};  
  
if (isDocker) {  
	_proxy['/remote-client'] = {  
		target: remoteClientTarget,  
		changeOrigin: true,  
		secure: false,  
		rewrite: path => path.replace(/^\/remote-client/, '')  
	};  
}  

export default defineConfig({
	plugins: [uiCssHmrPlugin()],
	root: './',
	base: './',
	resolve: {
		alias: {
			App: path.resolve(__dirname, './src/App'),
			Audio: path.resolve(__dirname, './src/Audio'),
			Controls: path.resolve(__dirname, './src/Controls'),
			Core: path.resolve(__dirname, './src/Core'),
			DB: path.resolve(__dirname, './src/DB'),
			Engine: path.resolve(__dirname, './src/Engine'),
			Loaders: path.resolve(__dirname, './src/Loaders'),
			Network: path.resolve(__dirname, './src/Network'),
			Plugins: path.resolve(__dirname, './src/Plugins'),
			Preferences: path.resolve(__dirname, './src/Preferences'),
			Renderer: path.resolve(__dirname, './src/Renderer'),
			UI: path.resolve(__dirname, './src/UI'),
			Utils: path.resolve(__dirname, './src/Utils'),
			Vendors: path.resolve(__dirname, './src/Vendors')
		}
	},
	optimizeDeps: {  
		include: ['bson', 'lodash', 'rijndael-js']  
	},
	test: {
		environment: 'jsdom',
		include: ['tests/**/*.test.js'],
		coverage: {  
			provider: 'v8',  
			reporter: ['text', 'html'],  
			include: ['src/**/*.js'],  
			exclude: ['src/Vendors/**']  
		}
	},
	build: {
		sourcemap: false, // Saves RAM
		minify: false, // Makes the build run much faster
		outDir: 'dist/Web',
		rollupOptions: {
			input: {
				main: path.resolve(__dirname, 'index.html')
			}
		}
	},
	server: {
		host: isDocker ? '0.0.0.0' : 'localhost', 
		port: 3000,
		open: !isDocker,
		cors: true,  
		...(isDocker && {  
			watch: {  
				usePolling: true, 
				interval: 1000  
			}  
		}),
		proxy: _proxy
	}	
});
