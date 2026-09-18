import { WebSocketServer } from 'ws';
import net from 'net';

// Parse command line arguments
const args = {};
for (let i = 2; i < process.argv.length; i++) {
	const arg = process.argv[i];
	if (arg.startsWith('-')) {
		const key = arg.replace(/^-+/, '');
		const val = process.argv[i + 1];
		if (val && !val.startsWith('-')) {
			args[key] = val;
			i++;
		} else {
			args[key] = true;
		}
	}
}

const port = parseInt(args.p || args.port || process.env.PORT || '5999', 10);
const redirectStr = args.r || args.redirect || '';
const redirects = {};

if (redirectStr) {
	redirectStr.split(',').forEach(pair => {
		const [src, dest] = pair.split('=');
		if (src && dest) {
			redirects[src.trim()] = dest.trim();
		}
	});
}

console.log(`[wsProxy] Listening on port ${port}`);
if (Object.keys(redirects).length > 0) {
	console.log('[wsProxy] Configured redirects:', redirects);
}

const wss = new WebSocketServer({ port });

wss.on('connection', (ws, req) => {
	const from = req.socket.remoteAddress;
	let target = req.url.slice(1); // Remove leading slash

	// Apply redirects
	if (redirects[target]) {
		console.log(`[wsProxy] Redirecting ${target} -> ${redirects[target]}`);
		target = redirects[target];
	}

	console.log(`[wsProxy] Connection request from ${from} to ${target}`);

	const parts = target.split(':');
	if (parts.length !== 2) {
		console.log(`[wsProxy] Invalid target format: ${target}`);
		ws.close();
		return;
	}

	const [host, portStr] = parts;
	const targetPort = parseInt(portStr, 10);

	const tcp = net.connect(targetPort, host, () => {
		console.log(`[wsProxy] Connected to target ${host}:${targetPort}`);
	});

	tcp.setNoDelay(true);

	ws.on('message', message => {
		if (tcp.writable) {
			tcp.write(message);
		}
	});

	tcp.on('data', data => {
		if (ws.readyState === ws.OPEN) {
			ws.send(data);
		}
	});

	const cleanup = () => {
		tcp.end();
		ws.close();
		console.log(`[wsProxy] Connection closed for ${target}`);
	};

	ws.on('close', cleanup);
	ws.on('error', cleanup);
	tcp.on('close', cleanup);
	tcp.on('error', err => {
		console.error(`[wsProxy] TCP Error for ${target}:`, err.message);
		cleanup();
	});
});
