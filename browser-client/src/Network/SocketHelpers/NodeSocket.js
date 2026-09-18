/**
 * Network/SocketHelpers/NodeSocket.js
 *
 * TCP socket via Electron preload (contextBridge).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault, AoShinHo
 */

class Socket {
	/**
	 * Electron TCP Socket
	 *
	 * @param {string} host
	 * @param {number} port
	 */
	constructor(host, port) {
		const self = this;
		this.connected = false;

		const sock = window.electronAPI.createSocket(host, port);
		this._socket = sock;

		sock.onConnect(() => {
			self.connected = true;
			self.onComplete(true);
		});

		sock.onError(() => {
			if (!self.connected) {
				self.onComplete(false);
			}
		});

		sock.onData(data => {
			self.onMessage(data);
		});

		sock.onClose(() => {
			self.connected = false;
			sock.destroy();
			if (self.onClose) {
				self.onClose();
			}
		});
	}

	/**
	 * @return {boolean} running in Electron
	 */
	static isSupported() {
		return !!window.electronAPI?.isElectron;
	}

	/**
	 * @param {ArrayBuffer} buffer
	 */
	send(buffer) {
		if (this.connected) {
			this._socket.write(buffer);
		}
	}

	/**
	 * Close connection
	 */
	close() {
		if (this.connected) {
			this._socket.end();
			this._socket.destroy();
			this.connected = false;
		}
	}
}
export default Socket;
