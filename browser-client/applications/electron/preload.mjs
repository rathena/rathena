// applications/electron/preload.mjs
import { contextBridge } from 'electron';
import net from 'node:net';

contextBridge.exposeInMainWorld('electronAPI', {
	isElectron: true,

	// ─── TCP Socket ─────────────────────────────────────────
	createSocket(host, port) {
		const socket = net.connect(port, host);

		return {
			onConnect(cb) {
				socket.on('connect', cb);
			},
			onError(cb) {
				socket.on('error', cb);
			},
			onData(cb) {
				socket.on('data', buf => cb(new Uint8Array(buf)));
			},
			onClose(cb) {
				socket.on('close', cb);
			},
			write(arrayBuffer) {
				socket.write(new Uint8Array(arrayBuffer));
			},
			end() {
				socket.end();
			},
			destroy() {
				socket.destroy();
			}
		};
	}
});
