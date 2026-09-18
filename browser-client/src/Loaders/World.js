/**
 * Loaders/World.js
 *
 * Loaders for Gravity .rsw file (Resource World)
 *
 * This file is part of ROBrowser, (http://www.robrowser.com/).
 *
 * @author Vincent Thibault
 */

import BinaryReader from 'Utils/BinaryReader.js';
import glMatrix from 'Utils/gl-matrix.js';

/**
 * World class loader
 *
 * @param {ArrayBuffer} data - optional
 */
class RSW {
	constructor(data) {
		this.sounds = [];
		this.lights = [];
		this.effects = [];
		this.models = [];

		if (data) {
			this.load(data);
		}
	}
	/**
	 * Files to load
	 */
	files = {
		buildnumber: null,
		ini: null,
		gnd: null,
		gat: null,
		src: null
	};

	/**
	 * Ground frustum culling
	 */
	ground = {
		top: -500,
		bottom: 500,
		left: -500,
		right: 500
	};

	/**
	 * Default water values (used to reset water on map change)
	 */
	static _defaultWater = {
		level: 0,
		type: 0,
		waveHeight: 0,
		waveSpeed: 0,
		wavePitch: 0,
		animSpeed: 0,
		splitWidth: 0,
		splitHeight: 0,
		images: new Array(32)
	};

	/**
	 * Water informations
	 */
	water = Object.assign({}, RSW._defaultWater);

	/**
	 * Light informations
	 */
	light = {
		longitude: 45,
		latitude: 45,
		diffuse: [1.0, 1.0, 1.0],
		ambient: [0.0, 0.0, 0.0],
		opacity: 1.0,
		direction: glMatrix.vec3.create()
	};

	/**
	 * Start loading RSW file
	 *
	 * @param {ArrayBuffer} data
	 */
	load(data) {
		let i, count;
		// Read header.
		const fp = new BinaryReader(data);
		const header = fp.readBinaryString(4);
		const version = fp.readByte() + fp.readByte() / 10;

		if (header != 'GRSW') {
			throw new Error(`RSW::load() - Invalid header "${header}", must be "GRSW"`);
		}

		if (version >= 2.5) {
			this.files.buildnumber = fp.readLong();
		}

		if (version >= 2.2) {
			fp.readByte();
		}

		// Read sub files.
		this.files.ini = fp.readBinaryString(40);
		this.files.gnd = fp.readBinaryString(40);
		this.files.gat = fp.readBinaryString(40);

		if (version >= 1.4) {
			this.files.src = fp.readBinaryString(40);
		}

		// Reset water to default values
		// A little hack, as changing maps, doesn't reset the water properties
		this.water = Object.assign({}, RSW._defaultWater);

		// Read water info.
		if (version < 2.6) {
			if (version >= 1.3) {
				this.water.level = fp.readFloat() / 5;
			} else {
				this.water.level = 0.0;
			}

			if (version >= 1.8) {
				this.water.type = fp.readLong();
				this.water.waveHeight = fp.readFloat() / 5;
				this.water.waveSpeed = fp.readFloat();
				this.water.wavePitch = fp.readFloat();
			} else {
				this.water.type = 0;
				this.water.waveHeight = 1.0;
				this.water.waveSpeed = 2.0;
				this.water.wavePitch = 50.0;
			}

			if (version >= 1.9) {
				this.water.animSpeed = fp.readLong();
			} else {
				this.water.animSpeed = 3;
			}
		}

		// Read lightmap.
		if (version >= 1.5) {
			this.light.longitude = fp.readLong();
			this.light.latitude = fp.readLong();
			this.light.diffuse = [fp.readFloat(), fp.readFloat(), fp.readFloat()];
			this.light.ambient = [fp.readFloat(), fp.readFloat(), fp.readFloat()];

			if (version >= 1.7) {
				this.light.opacity = fp.readFloat();
			}
		}

		// Read ground
		if (version >= 1.6) {
			this.ground.top = fp.readLong();
			this.ground.bottom = fp.readLong();
			this.ground.left = fp.readLong();
			this.ground.right = fp.readLong();
		}

		if (version >= 2.7) {
			count = fp.readLong();
			fp.seek(4 * count, SEEK_CUR); // Moves the file pointer forward by 4 * count bytes
		}

		// Read Object
		const models = this.models;
		const lights = this.lights;
		const sounds = this.sounds;
		const effects = this.effects;
		let m = 0,
			l = 0,
			s = 0,
			e = 0;

		// Allocate array (faster)
		count = fp.readLong();
		models.length = lights.length = sounds.length = effects.length = count;

		for (i = 0; i < count; ++i) {
			switch (fp.readLong()) {
				case 1:
					models[m++] = {
						name: version >= 1.3 ? fp.readBinaryString(40) : null,
						animType: version >= 1.3 ? fp.readLong() : 0,
						animSpeed: version >= 1.3 ? fp.readFloat() : 1.0,
						blockType: version >= 1.3 ? fp.readLong() : 0,
						UnknownByte: version >= 2.6 && this.files.buildnumber >= 186 ? fp.readByte() : 0,
						UnknownByte2: version >= 2.7 ? fp.readLong() : 0,
						filename: fp.readBinaryString(80),
						nodename: fp.readBinaryString(80),
						position: [fp.readFloat() / 5, fp.readFloat() / 5, fp.readFloat() / 5],
						rotation: [fp.readFloat(), fp.readFloat(), fp.readFloat()],
						scale: [fp.readFloat() / 5, fp.readFloat() / 5, fp.readFloat() / 5]
					};
					continue;

				case 2:
					lights[l++] = {
						name: fp.readBinaryString(80),
						pos: [fp.readFloat() / 5, fp.readFloat() / 5, fp.readFloat() / 5],
						color: [fp.readLong(), fp.readLong(), fp.readLong()],
						range: fp.readFloat()
					};
					continue;

				case 3:
					sounds[s++] = {
						name: fp.readBinaryString(80),
						file: fp.readBinaryString(80),
						pos: [fp.readFloat() / 5, fp.readFloat() / 5, fp.readFloat() / 5],
						vol: fp.readFloat(),
						width: fp.readLong(),
						height: fp.readLong(),
						range: fp.readFloat(),
						cycle: version >= 2.0 ? fp.readFloat() : 0.0
					};
					continue;

				case 4:
					effects[e++] = {
						name: fp.readBinaryString(80),
						pos: [fp.readFloat() / 5, fp.readFloat() / 5, fp.readFloat() / 5],
						id: fp.readLong(),
						delay: fp.readFloat() * 10,
						param: [fp.readFloat(), fp.readFloat(), fp.readFloat(), fp.readFloat()] // effectClass.apply(effect, effect.param) ?
					};
			}
		}

		// Remove empty content
		models.length = m;
		sounds.length = s;
		lights.length = l;
		effects.length = e;

		// skip quadtree (not used)
	}

	/**
	 * Compile RSW file
	 */
	compile() {
		return {
			water: this.water,
			light: this.light,
			sound: this.sounds,
			effect: this.effects
		};
	}
}
/**
 * Export
 */
export default RSW;
