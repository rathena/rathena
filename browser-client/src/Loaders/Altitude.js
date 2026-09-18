/**
 * Loaders/Altitude.js
 *
 * Loaders for Gravity .gat file (Ground Altitude)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import BinaryReader from 'Utils/BinaryReader.js';

/**
 * Altitude Constructor
 *
 * @param {ArrayBuffer} data
 */
class GAT {
	/**
	 * Cell known type
	 */
	static TYPE = {
		NONE: 1 << 0,
		WALKABLE: 1 << 1,
		WATER: 1 << 2,
		SNIPABLE: 1 << 3
	};

	/**
	 * Taken from *athena at src/map/map.c
	 * I don't know if it's a good source but it's a good idea to match this references for now
	 */
	static TYPE_TABLE = {
		0: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE, // walkable ground
		1: GAT.TYPE.NONE, // non-walkable ground
		2: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE, // ???
		3: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE | GAT.TYPE.WATER, // walkable water
		4: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE, // ???
		5: GAT.TYPE.SNIPABLE, // gat (snipable)
		6: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE, // ???

		/* Taken from Grf Editor */
		[-1]: GAT.TYPE.NONE, // NoGat (-1)
		[0x80000000]: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE, // Weird0 (Int32.MinValue)
		[0x80000001]: GAT.TYPE.NONE, // Weird1
		[0x80000002]: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE, // Weird2
		[0x80000003]: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE | GAT.TYPE.WATER, // Weird3
		[0x80000004]: GAT.TYPE.NONE, // Weird4
		[0x80000005]: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE, // Weird5
		[0x80000006]: GAT.TYPE.WALKABLE | GAT.TYPE.SNIPABLE, // Weird6
		[0x80000007]: GAT.TYPE.NONE, // Weird7
		[0x80000008]: GAT.TYPE.NONE, // Weird8
		[0x80000009]: GAT.TYPE.NONE // Weird9
	};
	constructor(data) {
		if (data) {
			this.load(data);
		}
	}

	/**
	 * Load a GAT file
	 *
	 * @param {ArrayBuffer} data
	 */
	load(data) {
		let i, count;

		const fp = new BinaryReader(data);
		const header = fp.readBinaryString(4);

		// Well, the file should be a gat file, noh ?
		if (header !== 'GRAT') {
			throw new Error(`GAT::load() - Invalid header "${header}", must be "GRAT"`);
		}

		// Load parameters
		const version = fp.readUByte() + fp.readUByte() / 10;
		const width = fp.readULong();
		const height = fp.readULong();
		const cells = new Float32Array(width * height * 5);

		// Load the cells
		for (i = 0, count = width * height; i < count; ++i) {
			// Creating x objects is too slow to send with postMessage...
			// So just generate a float32array (10 times faster)
			cells[i * 5 + 0] = fp.readFloat() * 0.2; // height 1
			cells[i * 5 + 1] = fp.readFloat() * 0.2; // height 2
			cells[i * 5 + 2] = fp.readFloat() * 0.2; // height 3
			cells[i * 5 + 3] = fp.readFloat() * 0.2; // height 4
			cells[i * 5 + 4] = GAT.TYPE_TABLE[fp.readULong()]; // type
		}

		// Exports
		this.width = width;
		this.height = height;
		this.cells = cells;
		this.version = version;
	}

	/**
	 * Compile GAT file
	 */
	compile() {
		// Return some usefulls things.
		return {
			cells: this.cells,
			width: this.width,
			height: this.height
		};
	}
}
/**
 * Export s
 */
export default GAT;
