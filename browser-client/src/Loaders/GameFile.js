/**
 * Loaders/GameFile.js
 *
 * Loaders for Gravity .grf file (Game RO File)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import GameFileDecrypt from './GameFileDecrypt.js';
import BinaryReader from 'Utils/BinaryReader.js';
import Struct from 'Utils/Struct.js';
import Inflate from 'Utils/Inflate.js';
import TextEncoding from 'Utils/CodepageManager.js';

/* global process */
let fs = null;

const isElectron = typeof process !== 'undefined' && process.versions?.electron;

if (isElectron) {
	try {
		// eslint-disable-next-line
		const req = Function('return require')();
		fs = req('fs');
	} catch {
		//ignore error
	}
}

/**
 * Extensions that should skip full encryption (only header encryption)
 */
const SKIP_EXTENSIONS = /\.(gnd|gat|act|str)$/i;

/**
 * GRF Constructor
 *
 * @param {File} data
 */
class GRF {
	constructor(data) {
		if (data) {
			this.load(data);
		}
	}

	/**
	 * GRF Constants
	 */
	static VERSION_200 = 0x200;
	static VERSION_300 = 0x300;
	static SIG_MAGIC = 'Master of Magic';
	static SIG_EH3 = 'Event Horizon';

	static FILELIST_TYPE_FILE = 0x01; // entry is a file
	static FILELIST_TYPE_ENCRYPT_MIXED = 0x02; // encryption mode 0 (header DES + periodic DES/shuffle)
	static FILELIST_TYPE_ENCRYPT_HEADER = 0x04; // encryption mode 1 (header DES only)

	/**
	 * GRF Structures
	 */
	static struct_header = new Struct(
		'unsigned char signature[15]',
		'unsigned char key[15]',
		'unsigned long file_table_offset',
		'unsigned long skip',
		'unsigned long filecount',
		'unsigned long version'
	);

	static struct_table = new Struct('unsigned long pack_size', 'unsigned long real_size');

	static struct_entry = new Struct(
		'unsigned long pack_size',
		'unsigned long length_aligned',
		'unsigned long real_size',
		'unsigned char type',
		'unsigned long offset'
	);

	/**
	 * GRF METHODs
	 */
	file = null;
	reader = null;
	header = null;
	table = null;

	/**
	 * Loading GRF
	 *
	 * @param {File} file
	 */
	load(file) {
		// Global object
		this.file = file;
		this.reader = new FileReaderSync();

		// Local object
		const reader = this.reader;
		let i, count;

		// Helper
		file.slice = file.slice || file.webkitSlice || file.mozSlice;
		reader.load = (start, len) => {
			if (fs && file.fd) {
				const buf = Buffer.alloc(len);
				fs.readSync(file.fd, buf, 0, len, start);
				return new Uint8Array(buf).buffer;
			}
			return reader.readAsArrayBuffer(file.slice(start, start + len));
		};

		// Check if file has enought content.
		if (file.size < GRF.struct_header.size) {
			throw new Error('GRF::load() - Not enough bytes to be a valid GRF');
		}

		// Read the header
		let buffer = reader.load(0, GRF.struct_header.size);
		let fp = new BinaryReader(buffer);
		const header = fp.readStruct(GRF.struct_header);

		header.signature = String.fromCharCode.apply(null, header.signature);
		const nullPos = header.signature.indexOf('\0');
		if (nullPos !== -1) {
			header.signature = header.signature.substr(0, nullPos);
		}

		// Check file header
		if (header.signature !== GRF.SIG_MAGIC && header.signature !== GRF.SIG_EH3) {
			throw new Error(
				`GRF::load() - Incorrect header "${header.signature}", must be "Master of Magic" or "Event Horizon".`
			);
		}

		// Support 0x200 and 0x300
		if (header.version !== GRF.VERSION_200 && header.version !== GRF.VERSION_300) {
			throw new Error(
				`GRF::load() - Incorrect version "0x${parseInt(header.version, 10).toString(16)}", just support version "0x200" and "0x300"`
			);
		}

		// Version 0x300 specific header read
		// pack_offset(8) + filecount(4) + version(4) starting at offset 30
		if (header.version === GRF.VERSION_300) {
			fp.seek(30, SEEK_SET);
			header.file_table_offset = fp.readUInt64();
			header.filecount = fp.readUInt();
			header.realfilecount = header.filecount;
		} else {
			header.filecount -= header.skip + 7;
			header.realfilecount = header.filecount;
		}

		if (header.file_table_offset + GRF.struct_header.size > file.size || header.file_table_offset < 0) {
			throw new Error(
				`GRF::load() - Can't jump to table list (${header.file_table_offset}), file length: ${file.size}`
			);
		}

		// Load Table Info
		// 0x300 has a unknown Int32 field before the fileTable
		let table_offset = header.file_table_offset + GRF.struct_header.size;
		if (header.version === GRF.VERSION_300) {
			table_offset += 4;
		}

		buffer = reader.load(table_offset, GRF.struct_table.size);
		fp = new BinaryReader(buffer);
		const table = fp.readStruct(GRF.struct_table);

		// Load Table Data
		buffer = reader.load(table_offset + GRF.struct_table.size, table.pack_size);
		const data = new Uint8Array(buffer);
		const out = new Uint8Array(table.real_size);

		// Uncompress data
		new Inflate(data).getBytes(out);
		this.index = {};
		// Load entries
		const entries = loadEntries(out, header.realfilecount, header.version);

		// Store table data (used for regex search in tablelist)
		// Set filename to lowercase (case insensitive in official client)
		table.data = '';
		for (i = 0, count = entries.length; i < count; ++i) {
			table.data += `${entries[i].filename}\0`;
			entries[i].filename = entries[i].filename.toLowerCase();
			// Store index for quick search
			this.index[entries[i].filename] = entries[i];
		}

		this.header = header;
		this.entries = entries;
		this.table = table;
	}

	/**
	 * Decode entry to return its content
	 *
	 * @param {ArrayBuffer}
	 * @param {Entry}
	 * @param {function} callback
	 */
	decodeEntry(buffer, entry, callback) {
		let out;
		const data = new Uint8Array(buffer);
		const isEncrypted = entry.type !== GRF.FILELIST_TYPE_FILE;
		let handled = false;

		// Decode the file
		if (entry.type & GRF.FILELIST_TYPE_ENCRYPT_MIXED) {
			if (SKIP_EXTENSIONS.test(entry.filename)) {
				GameFileDecrypt.decodeHeader(data, entry.length_aligned);
			} else {
				GameFileDecrypt.decodeFull(data, entry.length_aligned, entry.pack_size);
			}
			handled = true;
		} else if (entry.type & GRF.FILELIST_TYPE_ENCRYPT_HEADER) {
			GameFileDecrypt.decodeHeader(data, entry.length_aligned);
			handled = true;
		}

		if (isEncrypted && !handled) {
			console.warn(
				`Unsupported encryption flag (${entry.type}) for file ${entry.filename}. This usually requires a custom decryption key.`
			);
			return;
		}

		if (
			data[0] !== 0 &&
			(data[0] !== 0x78 || (data[1] !== 0x9c && data[1] !== 0x01 && data[1] !== 0xda && data[1] !== 0x5e))
		) {
			console.warn(`GRF: file "${entry.filename}" is using a new encryption method which is not supported.`);
			return;
		}

		// Uncompress
		try {
			out = new Uint8Array(entry.real_size);
			new Inflate(data).getBytes(out);

			callback(out.buffer);
		} catch (e) {
			console.error('Failed to decode entry', entry.filename, 'due to', e);
		}
	}

	/**
	 * Search a file in the GRF
	 *
	 * @param {string} filename
	 */
	search(filename) {
		return this.index[filename] || null;
	}

	/**
	 * Get a file content from GRF
	 *
	 * @param {string} filename
	 * @param {function} callback
	 */
	getFile(filename, callback) {
		// Not case sensitive...
		const path = filename.toLowerCase();
		let blob;
		let reader;

		const entry = this.search(path);

		// If filename is find in GRF table list
		if (entry) {
			// Directory ?
			if (!(entry.type & GRF.FILELIST_TYPE_FILE)) {
				return false;
			}

			if (fs && this.file.fd) {
				const buffer = Buffer.alloc(entry.length_aligned);
				fs.readSync(this.file.fd, buffer, 0, entry.length_aligned, entry.offset + GRF.struct_header.size);
				this.decodeEntry(new Uint8Array(buffer).buffer, entry, callback);
				return true;
			}

			blob = this.file.slice(
				entry.offset + GRF.struct_header.size,
				entry.offset + GRF.struct_header.size + entry.length_aligned
			);

			// Load into memory
			if (self.FileReader) {
				reader = new FileReader();
				reader.onload = () => {
					this.decodeEntry(reader.result, entry, callback);
				};
				reader.readAsArrayBuffer(blob);
			}

			// Firefox doesn't seems to support FileReader in web worker
			else {
				reader = new FileReaderSync();
				this.decodeEntry(reader.readAsArrayBuffer(blob), entry, callback);
			}

			return true;
		}

		return false;
	}
}

/**
 * Load entries
 * Note: this function is quiet intensive, BinaryReader is slowing down
 * the process and generate too much memory to garbage (GC pause of 6sec).
 *
 * @param {Uint8Array} content table
 * @param {number} file count
 * @param {number} grf version
 */
function loadEntries(out, count, version) {
	// Read all entries
	let i, pos, start, end;
	const entries = new Array(count);

	for (i = 0, pos = 0; i < count; ++i) {
		start = pos;
		while (out[pos]) {
			pos++;
		}
		end = pos;
		pos++;

		entries[i] = {
			filename: TextEncoding.decode(out.subarray(start, end), 'utf-8'),
			pack_size: out[pos++] | (out[pos++] << 8) | (out[pos++] << 16) | (out[pos++] << 24),
			length_aligned: out[pos++] | (out[pos++] << 8) | (out[pos++] << 16) | (out[pos++] << 24),
			real_size: out[pos++] | (out[pos++] << 8) | (out[pos++] << 16) | (out[pos++] << 24),
			type: out[pos++]
		};

		if (version === GRF.VERSION_300) {
			entries[i].offset = (out[pos++] | (out[pos++] << 8) | (out[pos++] << 16) | (out[pos++] << 24)) >>> 0;
			entries[i].offset +=
				(out[pos++] | (out[pos++] << 8) | (out[pos++] << 16) | (out[pos++] << 24)) * 0x100000000;
		} else {
			entries[i].offset = (out[pos++] | (out[pos++] << 8) | (out[pos++] << 16) | (out[pos++] << 24)) >>> 0;
		}
	}

	return entries;
}

/**
 * Export
 */
export default GRF;
