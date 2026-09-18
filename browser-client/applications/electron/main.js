import { app, BrowserWindow, protocol } from 'electron';
import { URL, fileURLToPath } from 'node:url';
import fs from 'node:fs';
import path from 'node:path';
import process from 'node:process';

const __dirname = path.dirname(fileURLToPath(import.meta.url));

app.commandLine.appendSwitch('enable-webgl');
app.commandLine.appendSwitch('ignore-gpu-blacklist');
app.commandLine.appendSwitch('disable-raf-throttling');
app.commandLine.appendSwitch('disable-gpu-vsync');
app.commandLine.appendSwitch('disable-frame-rate-limit');

const projectRoot = path.resolve(__dirname, '..', '..');

// MIME types required for ES module loading — Chromium enforces strict
// MIME checking for <script type="module"> and dynamic import().
const MIME_TYPES = {
	'.html': 'text/html',
	'.js': 'text/javascript',
	'.mjs': 'text/javascript',
	'.css': 'text/css',
	'.json': 'application/json',
	'.png': 'image/png',
	'.jpg': 'image/jpeg',
	'.gif': 'image/gif',
	'.bmp': 'image/bmp',
	'.svg': 'image/svg+xml',
	'.wav': 'audio/wav',
	'.mp3': 'audio/mpeg',
	'.wasm': 'application/wasm',
	'.tga': 'image/targa',
	'.lub': 'application/octet-stream',
	'.lua': 'application/octet-stream',
	'.spr': 'application/octet-stream',
	'.act': 'application/octet-stream',
	'.grf': 'application/octet-stream'
};

// Register custom protocol so ES modules work without CORS issues.
// file:// blocks <script type="module"> imports; app:// serves them
// with a proper origin and correct MIME types.
protocol.registerSchemesAsPrivileged([
	{
		scheme: 'app',
		privileges: {
			standard: true,
			secure: true,
			supportFetchAPI: true,
			corsEnabled: true
		}
	}
]);

function createWindow() {
	const win = new BrowserWindow({
		width: 1024,
		height: 768,
		fullscreen: false,
		frame: true,
		icon: path.join(__dirname, 'icon.png'),
		webPreferences: {
			preload: path.join(__dirname, 'preload.mjs'),
			contextIsolation: true,
			nodeIntegration: false,
			nodeIntegrationInWorker: true,
			sandbox: false
		}
	});

	if (process.argv.includes('--dev')) {
		process.env.ELECTRON_DISABLE_SECURITY_WARNINGS = 'true';
		win.loadURL('http://localhost:3000/applications/electron/index.html');
		win.webContents.openDevTools();
	} else {
		win.loadURL('app://localhost/applications/electron/index.html');
	}

	win.on('close', () => {
		app.exit(0);
	});
}

app.whenReady().then(() => {
	// Also emulates Vite's ?raw suffix: when a module imports e.g.
	// './Intro.html?raw', we return a JS module that exports the file
	// content as a string (instead of serving raw HTML/CSS which Chromium
	// rejects as non-JS MIME for module scripts).
	// Handle app:// requests by reading local files with correct MIME types.
	protocol.handle('app', request => {
		const url = new URL(request.url);
		const isRaw = url.searchParams.has('raw');
		const relativePath = decodeURIComponent(url.pathname).replace(/^\/+/, '');
		const filePath = path.normalize(path.join(projectRoot, relativePath));

		// Prevent path traversal outside project root
		if (!filePath.startsWith(projectRoot + path.sep) && filePath !== projectRoot) {
			console.error(`[app://] Forbidden (path traversal): ${request.url} → ${filePath}`);
			return new Response('Forbidden', { status: 403 });
		}

		try {
			if (!fs.existsSync(filePath)) {
				console.error(`[app://] 404: ${request.url} → ${filePath}`);
				return new Response('Not Found', { status: 404 });
			}

			const data = fs.readFileSync(filePath);

			// ?raw → wrap file content in a JS module (Vite compat)
			if (isRaw) {
				const text = data.toString('utf-8');
				const escaped = JSON.stringify(text);
				const js = `export default ${escaped};`;
				return new Response(js, {
					headers: { 'Content-Type': 'text/javascript' }
				});
			}

			const ext = path.extname(filePath).toLowerCase();
			const mimeType = MIME_TYPES[ext] || 'application/octet-stream';
			let body = data;

			// Transform JS files to resolve bare specifiers (roBrowser aliases)
			if (mimeType === 'text/javascript' && !isRaw) {
				let content = data.toString('utf8');
				content = content.replace(
					/from\s+['"](Core|Loaders|Utils|Network|DB|Renderer|UI|App|Audio|Controls|Plugins|Preferences|Engine|Vendors)\/(.*?)['"]/g,
					"from '/src/$1/$2'"
				);
				body = Buffer.from(content);
			}

			return new Response(body, {
				headers: { 'Content-Type': mimeType }
			});
		} catch (e) {
			console.error(`[app://] Error: ${request.url} →`, e);
			return new Response(e.message, { status: 500 });
		}
	});

	createWindow();
});

app.on('window-all-closed', () => {
	if (process.platform !== 'darwin') {
		app.quit();
		// Fallback: force exit if quit takes too long
		setTimeout(() => process.exit(0), 1000);
	}
});

app.on('activate', () => {
	if (BrowserWindow.getAllWindows().length === 0) {
		createWindow();
	}
});
