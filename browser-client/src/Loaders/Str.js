/**
 * Loaders/Str.js
 *
 * Loaders for Gravity .str file (effects file)
 * It's basically a .ezv file compiled to binary (except ezv file are version 0x95, str are 0x94).
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import BinaryReader from 'Utils/BinaryReader.js';

/**
 * Frame structure
 *
 * @param {BinaryReader} fp
 */
class STRAnimation {
	constructor(fp) {
		this.frame = fp.readLong();
		this.type = fp.readULong();
		this.pos = new Float32Array([fp.readFloat(), fp.readFloat()]);
		this.uv = new Float32Array([
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat()
		]);
		this.xy = new Float32Array([
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat(),
			fp.readFloat()
		]);
		this.aniframe = fp.readFloat();
		this.anitype = fp.readULong();
		this.delay = fp.readFloat();
		this.angle = fp.readFloat() / (1024 / 360);
		this.color = new Float32Array([
			fp.readFloat() / 255.0,
			fp.readFloat() / 255.0,
			fp.readFloat() / 255.0,
			fp.readFloat() / 255.0
		]);
		this.srcalpha = fp.readULong();
		this.destalpha = fp.readULong();
		this.mtpreset = fp.readULong();
	}
}

/**
 * Layer structure
 *
 * @param {BinaryReader} fp
 * @param {string} texturePath
 */
class STRLayer {
	constructor(fp, texturePath) {
		let i;

		this.texcnt = fp.readLong();
		this.texname = new Array(this.texcnt);

		for (i = 0; i < this.texcnt; ++i) {
			this.texname[i] = `data\\texture\\effect\\${texturePath}${fp.readBinaryString(128)}`;
		}

		this.anikeynum = fp.readLong();
		this.animations = new Array(this.anikeynum);

		for (i = 0; i < this.anikeynum; ++i) {
			this.animations[i] = new STRAnimation(fp);
		}
	}
}
/**
 * Str class loader
 *
 * @param {ArrayBuffer} data - optional
 * @param {string} texturePath - optional
 */
class STR {
	constructor(data, texturePath) {
		this.version = 0.0;
		this.texturePath = texturePath ?? '';

		if (data) {
			this.load(data);
		}
	}
	/**
	 * Parse a STR file
	 *
	 * @param {ArrayBuffer} data
	 */
	load(data) {
		let i;

		const fp = new BinaryReader(data);
		this.header = fp.readBinaryString(4);

		if (this.header !== 'STRM') {
			throw new Error(`STR::load() - Incorrect header "${this.header}", must be "STRM"`);
		}

		this.version = fp.readULong();

		if (this.version !== 0x94) {
			throw new Error(`STR - Invalid version "${this.version}", not supported`);
		}

		this.fps = fp.readULong();
		this.maxKey = fp.readULong();
		this.layernum = fp.readULong();
		fp.seek(16, SEEK_CUR); // display, group, type, ... ?

		this.layers = new Array(this.layernum);

		for (i = 0; i < this.layernum; ++i) {
			this.layers[i] = new STRLayer(fp, this.texturePath);
		}
	}
}

/**
 * Export
 */
export default STR;
