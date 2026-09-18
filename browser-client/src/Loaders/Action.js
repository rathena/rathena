/**
 * Loaders/Action.js
 *
 * Loaders for Gravity .act file (Action)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import BinaryReader from 'Utils/BinaryReader.js';

/**
 * Action class loader
 *
 * @param {ArrayBuffer} data - optional
 */
class ACT {
	constructor(data) {
		this.fp = null;
		this.version = 0.0;
		this.actions = [];
		this.sounds = [];

		if (data) {
			this.load(data);
		}
	}

	/**
	 * Parse an ACT file
	 *
	 * @param {ArrayBuffer} data
	 */
	load(data) {
		let i, count;

		this.fp = new BinaryReader(data);
		this.header = this.fp.readBinaryString(2);

		if (this.header !== 'AC') {
			throw new Error(`ACT::load() - Incorrect header "${this.header}", must be "AC"`);
		}

		this.version = this.fp.readUByte() / 10 + this.fp.readUByte();
		this.readActions();

		if (this.version >= 2.1) {
			// Sound
			count = this.fp.readLong();
			this.sounds.length = count;

			for (i = 0; i < count; ++i) {
				this.sounds[i] = this.fp.readBinaryString(40);
			}

			// Delay
			if (this.version >= 2.2) {
				for (i = 0, count = this.actions.length; i < count; ++i) {
					this.actions[i].delay = this.fp.readFloat() * 25;
				}
			}
		}
	}

	/**
	 * Load Action part of ACT file
	 */
	readActions() {
		let i;
		const count = this.fp.readUShort();
		const actions = this.actions;

		// Unknown bytes...
		this.fp.seek(10, SEEK_CUR);
		actions.length = count;

		for (i = 0; i < count; ++i) {
			actions[i] = {
				animations: this.readAnimations(),
				delay: 150
			};
		}
	}

	/**
	 *	Load Animation part in ACT file
	 */
	readAnimations() {
		const fp = this.fp;
		let i;
		const count = fp.readULong();
		const anim = new Array(count);

		for (i = 0; i < count; ++i) {
			// Unknown bytes
			fp.seek(32, SEEK_CUR);
			anim[i] = this.readLayers();
		}

		return anim;
	}

	/**
	 * Load ACT Layers
	 */
	readLayers() {
		const fp = this.fp;
		let count = fp.readULong();
		const layers = new Array(count);
		let layer;
		const version = this.version;
		let i;

		for (i = 0; i < count; ++i) {
			layer = layers[i] = {
				pos: [fp.readLong(), fp.readLong()],
				index: fp.readLong(),
				is_mirror: fp.readLong(),
				scale: [1.0, 1.0],
				color: [1.0, 1.0, 1.0, 1.0],
				angle: 0,
				spr_type: 0,
				width: 0,
				height: 0
			};

			if (version >= 2.0) {
				layer.color[0] = fp.readUByte() / 255;
				layer.color[1] = fp.readUByte() / 255;
				layer.color[2] = fp.readUByte() / 255;
				layer.color[3] = fp.readUByte() / 255;
				layer.scale[0] = fp.readFloat();
				layer.scale[1] = version <= 2.3 ? layer.scale[0] : fp.readFloat();
				layer.angle = fp.readLong();
				layer.spr_type = fp.readLong();

				if (version >= 2.5) {
					layer.width = fp.readLong();
					layer.height = fp.readLong();
				}
			}
		}

		const sound = version >= 2.0 ? fp.readLong() : -1;
		const pos = [];

		if (version >= 2.3) {
			count = fp.readLong();
			pos.length = count;

			for (i = 0; i < count; ++i) {
				fp.seek(4, SEEK_CUR); // Unknown
				pos[i] = { x: fp.readLong(), y: fp.readLong() };
				fp.seek(4, SEEK_CUR); // Unknown
			}
		}

		return {
			layers: layers,
			sound: sound,
			pos: pos
		};
	}

	/**
	 * Make it transferable in worker context
	 */
	compile() {
		return {
			actions: this.actions,
			sounds: this.sounds
		};
	}
}
/**
 * Export
 */
export default ACT;
