/**
 * Utils/BinaryWriter.js
 *
 * BinaryWriter Helper
 *
 * Helper to build binary data (write sockets, files, ...)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import TextEncoding from 'Utils/CodepageManager.js';

/**
 * Extend DataView to add position
 *
 * @param {number} offset
 * @param {Array} value (x, y)
 * @param {boolean} littleEndian (if true use LE encode order)
 */
DataView.prototype.setPos = function SetPos(offset, value, littleEndian) {
	const x = value[0];
	const y = value[1];

	if (littleEndian) {
		this.setInt8(offset + 0, x >> 2, true);
		this.setInt8(offset + 1, ((x % 4) << 6) | (y >> 4), true);
		this.setInt8(offset + 2, (y % 16) << 4, true);
	} else {
		this.setInt8(offset + 2, x >> 2, true);
		this.setInt8(offset + 1, ((x % 4) << 6) | (y >> 4), true);
		this.setInt8(offset + 0, (y % 16) << 4, true);
	}
};

/**
 * Extend DataView to get setString method
 *
 * @param {number} offset
 * @param {string} str
 * @param {number} len
 */
DataView.prototype.setString = function SetString(offset, str, len) {
	if (len) {
		str = String(str).substr(0, len);
	}

	let i, count;
	const data = TextEncoding.encode(str, 'utf-8');

	// fuck it, need to rebuild the buffer
	if (!len && data.length > str.length) {
		// make sure to not use it in this case, else it will bug :(
	}

	for (i = 0, count = len || data.length; i < count; ++i) {
		this.setUint8(offset + i, data[i]);
	}
};

/**
 * Extend DataView to get setBinaryString method
 *
 * @param {number} offset
 * @param {string} str
 * @param {number} len
 */
DataView.prototype.setBinaryString = function SetBinaryString(offset, str, len) {
	if (len) {
		str = String(str).substr(0, len);
	}

	let i, count;

	for (i = 0, count = str.length; i < count; ++i) {
		this.setUint8(offset + i, str.charCodeAt(i) & 0xff);
	}
};

/**
 * BinaryWriter
 * Helper to write binary data
 *
 * @param {number} length
 */
function BinaryWriter(length) {
	this.buffer = new ArrayBuffer(length);
	this.view = new DataView(this.buffer);
	this.offset = 0;
}

/**
 * Write Int8 to buffer
 *
 * @param {number} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setInt8 =
	BinaryWriter.prototype.writeChar =
	BinaryWriter.prototype.writeByte =
		function setInt8(value) {
			this.view.setInt8(this.offset++, value, true);
			return this;
		};

/**
 * Write Uint8 to buffer
 *
 * @param {number} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setUint8 =
	BinaryWriter.prototype.writeUChar =
	BinaryWriter.prototype.writeUByte =
		function setUint8(value) {
			this.view.setUint8(this.offset++, value, true);
			return this;
		};

/**
 * Write Int16 to buffer
 *
 * @param {number} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setInt16 = BinaryWriter.prototype.writeShort = function setInt16(value) {
	this.view.setInt16(this.offset, value, true);
	this.offset += 2;
	return this;
};

/**
 * Write Uint16 to buffer
 *
 * @param {number} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setUint16 = BinaryWriter.prototype.writeUShort = function setUint16(value) {
	this.view.setUint16(this.offset, value, true);
	this.offset += 2;
	return this;
};

/**
 * Write Int32 to buffer
 *
 * @param {number} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setInt32 =
	BinaryWriter.prototype.writeInt =
	BinaryWriter.prototype.writeLong =
		function setInt32(value) {
			this.view.setInt32(this.offset, value, true);
			this.offset += 4;
			return this;
		};

/**
 * Write Uint32 to buffer
 *
 * @param {number} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setUint32 =
	BinaryWriter.prototype.writeUInt =
	BinaryWriter.prototype.writeULong =
		function setUint32(value) {
			this.view.setUint32(this.offset, value, true);
			this.offset += 4;
			return this;
		};

/**
 * Write Uint64 to buffer (little endian)
 *
 * @param {number|bigint|object} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setUint64 = BinaryWriter.prototype.writeUInt64 = function setUint64(value) {
	let low = 0;
	let high = 0;

	if (typeof value === 'bigint') {
		low = Number(value & 0xffffffffn);
		high = Number((value >> 32n) & 0xffffffffn);
	} else if (typeof value === 'number') {
		if (!Number.isSafeInteger(value)) {
			console.warn(value, 'exceeds MAX_SAFE_INTEGER. Precision may be lost');
		}
		low = value >>> 0;
		high = Math.floor(value / 0x100000000) >>> 0;
	} else if (value && typeof value === 'object') {
		low = (value.low || 0) >>> 0;
		high = (value.high || 0) >>> 0;
	}

	this.view.setUint32(this.offset, low, true);
	this.view.setUint32(this.offset + 4, high, true);
	this.offset += 8;
	return this;
};

/**
 * Write Float32 to buffer
 *
 * @param {number} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setFloat32 = BinaryWriter.prototype.writeFloat = function setFloat32(value) {
	this.view.setFloat32(this.offset, value, true);
	this.offset += 4;
	return this;
};

/**
 * Write Float64 to buffer
 *
 * @param {number} value
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setFloat64 = BinaryWriter.prototype.writeDouble = function setFloat64(value) {
	this.view.setFloat64(this.offset, value, true);
	this.offset += 8;
	return this;
};

/**
 * Write String to buffer
 *
 * @param {string} str
 * @param {number} length
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setString = BinaryWriter.prototype.writeString = function setString(str, length) {
	if (length) {
		str = String(str).substr(0, length);
	}

	const data = TextEncoding.encode(str, 'utf-8');
	let i;
	const count = length || data.length;

	// TODO: make it better.
	// Fuck it ! Because of the charset the string is longer
	// So we need to create another buffer with the new size for it
	if (!length && data.length > str.length) {
		// create new buffer
		const uint8 = new Uint8Array(this.buffer.byteLength + (data.length - str.length));

		// copy the old one and use the new one
		uint8.set(new Uint8Array(this.buffer), 0);
		this.buffer = uint8.buffer;
		this.view = new DataView(this.buffer);

		// Modify packet size
		this.view.setInt16(2, uint8.length, true);
	}

	for (i = 0; i < count; ++i) {
		this.view.setUint8(this.offset + i, data[i]);
	}

	this.offset += length || count;
	return this;
};

/**
 * Write Binary String to buffer
 *
 * @param {string} str
 * @param {number} length
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setBinaryString = BinaryWriter.prototype.writeBinaryString = function setBinaryString(
	str,
	length
) {
	str = String(str);

	const len = length ? Math.min(length, str.length) : str.length;

	const buf = new Uint8Array(len);

	for (let i = 0; i < len; i++) {
		buf[i] = str.charCodeAt(i);
	}

	new Uint8Array(this.view.buffer, this.offset, len).set(buf);

	if (length && len < length) {
		new Uint8Array(this.view.buffer, this.offset + len, length - len).fill(0);
	}

	this.offset += length || len;
	return this;
};

/**
 * Write buffer to buffer
 *
 * @param {buffer} buffer
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setBuffer = BinaryWriter.prototype.writeBuffer = function setBuffer(buffer) {
	const data = new Uint8Array(this.buffer);
	data.set(new Uint8Array(buffer), this.offset);

	this.offset += buffer.byteLength;
	return this;
};

/**
 * Skip X bytes from buffer
 *
 * @param {number}
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.skip = function skip(count) {
	this.offset += count;
	return this;
};

/**
 * Write x, y pos to buffer
 *
 * @param {Array} (x, y)
 * @return {BinaryWriter}
 */
BinaryWriter.prototype.setPos = BinaryWriter.prototype.writePos = function setPos(xy) {
	const x = xy[0],
		y = xy[1];

	this.view.setInt8(this.offset++, x >> 2, true);
	this.view.setInt8(this.offset++, ((x % 4) << 6) | (y >> 4), true);
	this.view.setInt8(this.offset++, (y % 16) << 4, true);

	return this;
};

/**
 * Export
 */
export default BinaryWriter;
