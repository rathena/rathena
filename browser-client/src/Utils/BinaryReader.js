/**
 * Utils/BinaryReader.js
 *
 * BinaryReader Helper
 *
 * Helper to load/parse Binary data (sockets, files)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import Struct from './Struct.js';
import TextEncoding from 'Utils/CodepageManager.js';

/**
 * Binary Constant for BinaryReader::seek();
 * Export to global context
 */
export const SEEK_CUR = 1;
export const SEEK_SET = 2;
export const SEEK_END = 3;

const _global = typeof self !== 'undefined' ? self : window;
_global.SEEK_CUR = SEEK_CUR;
_global.SEEK_SET = SEEK_SET;
_global.SEEK_END = SEEK_END;

/**
 * BinaryReader
 *
 * @param mixed buffer
 * @param {number} start optional
 * @param {number} end optional
 */
export default function BinaryReader(mixed, start, end) {
	let buffer;

	if (typeof mixed === 'string') {
		const len = mixed.length;
		const uint8 = new Uint8Array(len);

		for (let i = 0; i < len; i++) {
			uint8[i] = mixed.charCodeAt(i);
		}

		buffer = uint8.buffer;
	} else if (mixed instanceof ArrayBuffer) {
		buffer = mixed;
	} else if (mixed instanceof Uint8Array) {
		buffer = mixed.buffer;
	} else {
		throw new Error('BinaryReader() - Undefined buffer type');
	}

	this.buffer = buffer;
	this.view = new DataView(buffer, start || 0, end || buffer.byteLength);
	this.offset = 0;
	this.length = (end || buffer.byteLength) - (start || 0);
}

/**
 * Read Int8 from buffer
 * @return int8
 */
BinaryReader.prototype.getInt8 =
	BinaryReader.prototype.readChar =
	BinaryReader.prototype.readByte =
		function getInt8() {
			return this.view.getInt8(this.offset++);
		};

/**
 * Read Uint8 from buffer
 * @return uint8
 */
BinaryReader.prototype.getUint8 =
	BinaryReader.prototype.readUChar =
	BinaryReader.prototype.readUByte =
		function getUint8() {
			return this.view.getUint8(this.offset++);
		};

/**
 * Read Int16 from buffer
 * @return int16
 */
BinaryReader.prototype.getInt16 = BinaryReader.prototype.readShort = function getInt16() {
	const data = this.view.getInt16(this.offset, true);
	this.offset += 2;

	return data;
};

/**
 * Read Uint16 from buffer
 * @return Uint16
 */
BinaryReader.prototype.getUint16 = BinaryReader.prototype.readUShort = function getUint16() {
	const data = this.view.getUint16(this.offset, true);
	this.offset += 2;

	return data;
};

/**
 * Read Int32 from buffer
 * @return int32
 */
BinaryReader.prototype.getInt32 =
	BinaryReader.prototype.readInt =
	BinaryReader.prototype.readLong =
		function getInt32() {
			const data = this.view.getInt32(this.offset, true);
			this.offset += 4;

			return data;
		};

/**
 * Read Uint32 from buffer
 * @return Uint32
 */
BinaryReader.prototype.getUint32 =
	BinaryReader.prototype.readUInt =
	BinaryReader.prototype.readULong =
		function getUint32() {
			const data = this.view.getUint32(this.offset, true);
			this.offset += 4;

			return data;
		};

/**
 * Read float32 from buffer
 * @return float32
 */
BinaryReader.prototype.getFloat32 = BinaryReader.prototype.readFloat = function getFloat32() {
	const data = this.view.getFloat32(this.offset, true);
	this.offset += 4;

	return data;
};

/**
 * Read Float64 from buffer
 * @return Float64
 */
BinaryReader.prototype.getFloat64 = BinaryReader.prototype.readDouble = function getFloat64() {
	const data = this.view.getFloat64(this.offset, true);
	this.offset += 8;

	return data;
};

/**
 * Read Int64 from buffer
 * @return Int64
 */
BinaryReader.prototype.getInt64 = BinaryReader.prototype.readInt64 = function readInt64() {
	const left = this.view.getUint32(this.offset, true);
	const right = this.view.getInt32(this.offset + 4, true);

	// combine the two 32-bit values
	const combined = left + 2 ** 32 * right; // little endian

	if (!Number.isSafeInteger(combined)) {
		console.warn(combined, 'exceeds MAX_SAFE_INTEGER. Precision may be lost');
	}

	this.offset += 8;

	return combined;
};

/**
 * Read UInt64 from buffer
 * @return UInt64
 */
BinaryReader.prototype.getUInt64 = BinaryReader.prototype.readUInt64 = function readUInt64() {
	// split 64-bit number into two 32-bit parts
	const left = this.view.getUint32(this.offset, true);
	const right = this.view.getUint32(this.offset + 4, true);

	// combine the two 32-bit values
	const combined = left + 2 ** 32 * right; // little endian
	// const combined = 2**32*left + right; // big endian

	if (!Number.isSafeInteger(combined)) {
		console.warn(combined, 'exceeds MAX_SAFE_INTEGER. Precision may be lost');
	}

	this.offset += 8;

	return combined;
};

/**
 * Read Buffer position
 * @return {number}
 */
BinaryReader.prototype.tell = function tell() {
	return this.offset;
};

/**
 * Read string from buffer
 *
 * @param integer string length
 * @return string
 */
BinaryReader.prototype.getString = BinaryReader.prototype.readString = function getString(len) {
	const start = this.offset;
	const bytes = new Uint8Array(this.view.buffer, this.view.byteOffset + start, len);
	let realLen = len;
	for (let i = 0; i < len; i++) {
		if (bytes[i] === 0) {
			realLen = i;
			break;
		}
	}
	this.offset += len;
	return TextEncoding.decode(bytes.subarray(0, realLen), 'utf-8'); // default server charset
};

/**
 * Read binary string from buffer
 *
 * @param integer string length
 * @return string
 */
BinaryReader.prototype.getBinaryString = BinaryReader.prototype.readBinaryString = function getBinaryString(len) {
	const start = this.offset;
	const bytes = new Uint8Array(this.view.buffer, this.view.byteOffset + start, len);
	let realLen = len;
	for (let i = 0; i < len; i++) {
		if (bytes[i] === 0) {
			realLen = i;
			break;
		}
	}
	this.offset += len;
	return String.fromCharCode.apply(null, bytes.subarray(0, realLen));
};

/**
 * Structure reader in JS
 *
 * @param Struct
 */
BinaryReader.prototype.getStruct = BinaryReader.prototype.readStruct = function getStruct(struct) {
	if (!(struct instanceof Struct)) {
		throw new Error('BinaryReader::getStruct() - Invalid data as argument');
	}

	const list = struct._list;
	let name;
	const out = {};
	let current;
	let i, j;

	const keys = Object.keys(list);
	const count = keys.length;

	for (j = 0; j < count; ++j) {
		name = keys[j];
		current = list[name];

		if (current.count > 1) {
			out[name] = new Array(current.count);
			for (i = 0; i < current.count; ++i) {
				out[name][i] = this[current.func]();
			}
		} else {
			out[name] = this[current.func]();
		}
	}

	return out;
};

/**
 * Move cursor to another offset
 *
 * @param {number} index
 * @param {number} type - const SEEK_*
 */
BinaryReader.prototype.seek = function seek(index, type) {
	type = type || SEEK_SET;
	this.offset = type === SEEK_CUR ? this.offset + index : type === SEEK_END ? this.length + index : index;
};

// Old compatibility for pos and pos2
const bf_byteBuff = new ArrayBuffer(4);
const bf_wba = new Int8Array(bf_byteBuff);
const bf_wia = new Int32Array(bf_byteBuff);

/**
 * Read Position from Buffer
 *
 * @return array (pos_x, pos_y, direction)
 */
BinaryReader.prototype.getPos = BinaryReader.prototype.readPos = function getPos() {
	let p;

	bf_wba[2] = this.getUint8();
	bf_wba[1] = this.getUint8();
	bf_wba[0] = this.getUint8();
	bf_wba[3] = 0;

	p = 0 + bf_wia[0];
	const dir = p & 0x0f;
	p >>= 4;

	const y = p & 0x03ff;
	p >>= 10;

	const x = p & 0x03ff;

	return [x, y, dir];
};

/**
 * Read Position (walk) from buffer
 *
 * @return array ( from_x, from_y, to_x, to_y )
 */
BinaryReader.prototype.getPos2 = BinaryReader.prototype.readPos2 = function getPos2() {
	const a = this.getInt8();
	const b = this.getInt8();
	const c = this.getInt8();
	const d = this.getInt8();
	const e = this.getInt8();
	const f = this.getInt8();

	return [
		((a & 0xff) << 2) | ((b & 0xc0) >> 6), // x1
		((b & 0x3f) << 4) | ((c & 0xf0) >> 4), // y1
		((d & 0xfc) >> 2) | ((c & 0x0f) << 6), // x2
		((d & 0x03) << 8) | (e & 0xff), // y2
		(f & 0xf0) >> 4, // xcellpos aka subx0
		f & 0xf // ycellpos aka suby0
	];
};
