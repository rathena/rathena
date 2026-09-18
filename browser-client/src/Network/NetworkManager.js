/**
 * Network/NetworkManager.js
 *
 * Network Manager
 * Manage sockets and packets
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

// Load dependencies
import Configs from 'Core/Configs.js';
import BinaryReader from 'Utils/BinaryReader.js';
import PACKETVER from './PacketVerManager.js';
import PacketVersions from './PacketVersions.js';
import PacketRegister from './PacketRegister.js';
import PacketCrypt from './PacketCrypt.js';
import PacketLength from './PacketLength.js';
import WebSocket from './SocketHelpers/WebSocket.js';
import NodeSocket from './SocketHelpers/NodeSocket.js';

/**
 * Sockets list
 * @const {Socket[]}
 */
const _sockets = [];

/**
 * Custom socket factory for plugins
 * @type {function}
 */
let _socketFactory = null;

/**
 * Default socket factory - creates NodeSocket or WebSocket based on environment.
 * Custom factories can call this as a fallback.
 *
 * @param {string} host
 * @param {number} port
 * @return {Socket}
 */
function defaultSocketFactory(host, port) {
	const proxy = Configs.get('socketProxy', null);

	if (NodeSocket.isSupported()) {
		return new NodeSocket(host, port, proxy);
	}
	return new WebSocket(host, port, proxy);
}

/**
 * Current Socket
 * @type {Socket}
 */
let _socket = null;

/**
 * Buffer to use to read packets
 * @type {Uint8Array}
 */
let _save_buffer = null;

/**
 * Custom callback for disconnection
 * @type {function}
 */
let _onDisconnect = null;

/**
 * Defines if dump packets as hex string
 * @const {boolean}
 */
const packetDump = Configs.get('packetDump', false);

/**
 * Packets definition
 *
 * @param {string} name
 * @param {callback} struct - callback to parse the packet
 * @param {number} size - packet size
 */
function Packets(name, Struct, size) {
	this.name = name;
	this.Struct = Struct;
	this.size = size;
	this.callback = null;
}

/**
 * List of supported packets
 * @type {Packets[]}
 */
Packets.list = [];

/**
 * Connect to a server
 *
 * @param {string} host
 * @param {number} port
 * @param {function} callback once connected or not
 * @param {boolean} is zone server ?
 */
function connect(host, port, callback, isZone) {
	const socket = _socketFactory ? _socketFactory(host, port) : defaultSocketFactory(host, port);

	socket.isZone = !!isZone;
	socket.onClose = onClose;
	socket.onComplete = function onComplete(success) {
		let msg = 'Fail';
		let color = 'red';

		if (success) {
			msg = 'Success';
			color = 'green';

			// If current socket has ping, remove it
			if (_socket && _socket.ping) {
				clearInterval(_socket.ping);
			}

			socket.onMessage = receive;
			_sockets.push((_socket = socket));

			// Map server encryption
			if (isZone) {
				PacketCrypt.init();
			}
		}

		console.log('%c[Network] ' + msg + ' to connect to ' + host + ':' + port, 'font-weight:bold;color:' + color);
		callback.call(this, success);
	};
}

/**
 * Send a packet to the server
 *
 * @param Packet
 */
function sendPacket(Packet) {
	const pkt = Packet.build();

	if (packetDump) {
		const fp = new BinaryReader(pkt.buffer);
		const id = fp.readUShort();
		console.log(
			'%c[Network] Dump Send: \n%cPacket ID: 0x%s\nPacket Name: %s\nLength: %d\nContent:\n%s',
			'color:#007070',
			'color:inherit',
			id.toString(16),
			Packet.constructor.name,
			pkt.buffer.byteLength,
			utilsBufferToHexString(pkt.buffer).toUpperCase()
		);
	}

	console.log('%c[Network] Send:', 'color:#007070', Packet);

	// Encrypt packet
	if (_socket && _socket.isZone) {
		PacketCrypt.process(pkt.view);
	}

	send(pkt.buffer);
}

/**
 * Send buffer to the server
 *
 * @param {ArrayBuffer} buffer
 */
function send(buffer) {
	if (_socket) {
		_socket.send(buffer);
	}
}

/**
 * Register a Packet
 *
 * @param {number} id - packet UID
 * @param {function} struct - packet structure callback
 */
function registerPacket(id, Struct) {
	if (Struct) {
		Struct.id = id;
		Packets.list[id] = new Packets(Struct.name, Struct, Struct.size);
	} else {
		console.error('registerPacket: Struct is undefined for id: 0x' + parseInt(id, 10).toString(16));
	}
}

/**
 * Hook a Packet
 *
 * @param {object} packet
 * @param {function} callback to use packet
 */
function hookPacket(packet, callback) {
	if (!packet) {
		throw new Error('NetworkManager::HookPacket() - Invalid packet structure "' + JSON.stringify(packet) + '"');
	}

	if (!packet.id) {
		throw new Error('NetworkManager::HookPacket() - Packet not yet register "' + packet.name + '"');
	}

	Packets.list[packet.id].callback = callback;
}

/**
 * Force to read from a used version for the next receive data
 *
 * @param callback
 */
function read(callback) {
	read.callback = callback;
}

/**
 * Callback used for reading the data for the next buffer received from server
 * @type {function}
 */
read.callback = null;

/**
 * Received data from server
 *
 * @param {Uint8Array} buffer
 */
function receive(buf) {
	let id, packet;
	let length = 0;
	let offset = 0;
	let buffer;

	// Waiting for data ? concat the buffer
	if (_save_buffer) {
		const _data = new Uint8Array(_save_buffer.length + buf.byteLength);
		_data.set(_save_buffer, 0);
		_data.set(new Uint8Array(buf), _save_buffer.length);
		buffer = _data.buffer;
	} else {
		buffer = buf;
	}

	const fp = new BinaryReader(buffer);

	// Read hook
	if (read.callback) {
		read.callback(fp);
		read.callback = null;
	}

	// Read and parse packets
	while (fp.tell() < fp.length) {
		offset = fp.tell();

		// Not enough bytes...
		if (offset + 2 > fp.length) {
			_save_buffer = new Uint8Array(buffer, offset, fp.length - offset);
			return;
		}

		id = fp.readUShort();
		let packet_len = PacketLength.getPacketLength(id);
		packet_len = packet_len ? packet_len : fp.length - offset;
		// Packet not defined ?

		if (packet_len < 0) {
			// Not enough bytes...
			if (offset + 4 > fp.length) {
				_save_buffer = new Uint8Array(buffer, offset, fp.length - offset);
				return;
			}
			length = fp.readUShort();
		} else {
			length = packet_len;
		}

		offset += length;

		// Not enough bytes, need to wait for new buffer to read more.
		if (offset > fp.length) {
			offset = fp.tell() - (packet_len < 0 ? 4 : 2);
			_save_buffer = new Uint8Array(buffer, offset, fp.length - offset);
			return;
		}

		if (Packets.list[id]) {
			packet = Packets.list[id];

			if (packetDump) {
				const buffer_console = new Uint8Array(buffer, offset - length, length);
				console.log(
					'%c[Network] Dump Recv:\n%cPacket ID: 0x%s\nPacket Name: %s\nLength: %d\nContent:\n%s',
					'color:#900090',
					'color:inherit',
					id.toString(16),
					packet.name,
					length,
					utilsBufferToHexString(buffer_console).toUpperCase()
				);
			}

			// Parse packet
			//if (!packet.instance) {
			packet.instance = new packet.Struct(fp, offset);
			//}
			//else {
			//	packet.Struct.call(packet.instance, fp, offset); //this causes packet conflicts where the same type of packets following eachother copy the previous packet's variables with the previous values
			//}

			console.log('%c[Network] Recv:', 'color:#900090', packet.instance, packet.callback ? '' : '(no callback)');

			// Call controller
			if (packet.callback) {
				packet.callback(packet.instance);
			}
		} else {
			if (packetDump) {
				const unknown_buffer = new Uint8Array(buffer, offset - length, length);
				console.log(
					'%c[Network] Dump Recv:\n%cPacket ID: 0x%s\nPacket Name: [UNKNOWN]\nLength: %d\nContent:\n%s',
					'color:#900090',
					'color:inherit',
					id.toString(16),
					length,
					utilsBufferToHexString(unknown_buffer).toUpperCase()
				);
			}
			console.error(
				'[Network] Packet "%c0x%s%c" not registered, skipping %d bytes.',
				'font-weight:bold',
				id.toString(16),
				'font-weight:normal',
				length
			);
		}

		// Support for "0" type
		if (length) {
			fp.seek(offset, SEEK_SET);
		}
	}

	_save_buffer = null;
}

/**
 * Communication end
 * Server ask to close the socket
 */
function onClose() {
	const idx = _sockets.indexOf(this);

	if (this === _socket) {
		console.warn('[Network] Disconnect from server');

		if (_socket.ping) {
			clearInterval(_socket.ping);
		}

		if (_onDisconnect) {
			_onDisconnect();
		} else {
			import('UI/UIManager.js').then(UIManager => {
				UIManager.default.showErrorBox('Disconnected from Server.');
			});
		}
	}

	if (idx !== -1) {
		_sockets.splice(idx, 1);
	}
}

/**
 * Close connection with server
 * Is this needed ?
 */
function close() {
	let idx;

	if (_socket) {
		const s = _socket;
		_socket = null;

		s.close();

		if (s.isZone) {
			PacketCrypt.reset();
		}

		if (s.ping) {
			clearInterval(s.ping);
		}

		idx = _sockets.indexOf(s);
		if (idx !== -1) {
			_sockets.splice(idx, 1);
		}
	}
}

/**
 * Define a ping
 *
 * @param callback
 */
function setPing(callback) {
	if (_socket) {
		if (_socket.ping) {
			clearInterval(_socket.ping);
		}
		_socket.ping = setInterval(callback, 10000);

		while (_sockets.length > 1) {
			if (_socket !== _sockets[0]) {
				_sockets[0].close();
				_sockets.splice(0, 1);
			}
		}
	}
}

/**
 * Set a custom socket factory for plugins.
 * The factory receives (host, port) and returns a socket.
 * Call defaultSocketFactory(host, port) inside your factory as a fallback.
 *
 * @param {function|null} factory
 */
function setSocketFactory(factory) {
	_socketFactory = factory;
}

/**
 * Get back ip from long
 *
 * @param {number} long ip
 * @return {string} ip
 */
function utilsLongToIP(long) {
	const buf = new ArrayBuffer(4);
	const uint8 = new Uint8Array(buf);
	const uint32 = new Uint32Array(buf);
	uint32[0] = long;

	return Array.prototype.join.call(uint8, '.');
}

/**
 * Convert ArryBuffer into a hex string
 *
 * @param {ArrayBuffer} buffer
 */
function utilsBufferToHexString(buffer) {
	return [...new Uint8Array(buffer)].map(x => x.toString(16).padStart(2, '0') + ' ').join('');
}

/**
 * Export
 */
const Network = (function network() {
	let keys;
	let i, count;

	// Add packet version
	keys = Object.keys(PacketVersions);
	count = keys.length;

	for (i = 0; i < count; ++i) {
		PACKETVER.addSupport(keys[i], PacketVersions[keys[i]]);
	}

	// Register packets
	keys = Object.keys(PacketRegister);
	count = keys.length;

	for (i = 0; i < count; ++i) {
		registerPacket(keys[i], PacketRegister[keys[i]]);
	}

	return {
		sendPacket: sendPacket,
		send: send,
		setPing: setPing,
		connect: connect,
		hookPacket: hookPacket,
		close: close,
		read: read,
		set onDisconnect(callback) {
			_onDisconnect = callback;
		},
		get onDisconnect() {
			return _onDisconnect;
		},
		setSocketFactory: setSocketFactory,
		defaultSocketFactory: defaultSocketFactory,
		registerPacket: registerPacket,
		utils: {
			longToIP: utilsLongToIP
		}
	};
})();

export default Network;
